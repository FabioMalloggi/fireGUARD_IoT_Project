#include <stdio.h>
#include <stdbool.h>
#include "contiki.h"
#include "coap-engine.h"
#include "lib/sensor_sim.h"
#include "lib/smart_smoke_detector_utilities.h"
#include "lib/features_norm_constants.h"
#include "fire_detector.h"	// emlearn generated model

#include "os/dev/button-hal.h"
#include "os/dev/leds.h"
#include "sys/etimer.h"
#include "sys/log.h"

#define LOG_MODULE "SSD"
#define LOG_LEVEL LOG_LEVEL_APP

#define FIRE_ASSESSMENT_PERIOD 10 //sec

#define BUTTON_PRESS_TIME_TO_FIRE 2 //sec

/* ---------- Exposed CoAP Resources ---------- */
extern coap_resource_t  res_temp; 
extern coap_resource_t  res_hum;
extern coap_resource_t  res_pressure;
extern coap_resource_t  res_tvoc;
extern coap_resource_t  res_raw_h2; 
extern coap_resource_t  res_raw_ethanol;
extern coap_resource_t  res_pm1_0;
extern coap_resource_t  res_pm2_5;
extern coap_resource_t  res_nc0_5;

extern coap_resource_t res_status;

/* ------ Resources Inner Data Structure ------ */
extern unsigned int status; // environment state control variable

/* -------- Simulation Data Structures -------- */
static sensor_sim_t sensors; // Real Sensors Simulator

static bool simulate_fire_ignition = false; // Fire Env. State Simulator
static bool simulate_hazard_condition = false; // Hazard Env. State Simulator

static bool water_sprinkler = false; // Water Sprinkler Fire Suppression Simulator

static button_hal_button_t *btn; // Fire/Hazard Conditions Input Simulator

/* ----------- Logic Data Structure ----------- */
static struct etimer e_sensing_timer;


static void activate_all_resources(void){
  coap_activate_resource(&res_temp, "temp");
  coap_activate_resource(&res_hum,  "hum");
  coap_activate_resource(&res_pressure,  "pressure");
  coap_activate_resource(&res_tvoc,  "tvoc");
  coap_activate_resource(&res_raw_h2,  "raw_h2");
  coap_activate_resource(&res_raw_ethanol,  "raw_ethanol");
  coap_activate_resource(&res_pm1_0,  "pm1_0");
  coap_activate_resource(&res_pm2_5,  "pm2_5");
  coap_activate_resource(&res_nc0_5,  "nc0_5");
  coap_activate_resource(&res_status, "status");
}

static bool fire_detected(sensor_sim_t *s) {
    // input vector
    float features[9];       
    /* -------- Feature Normalization --------- */
    features[0] = (s->temperature - MEAN_TEMP) / STD_DEV_TEMP;
    features[1] = (s->humidity - MEAN_HUMIDITY) / STD_DEV_HUMIDITY;
    features[2] = ((float)s->tvoc - MEAN_TVOC) / STD_DEV_TVOC;
    features[3] = ((float)s->raw_h2 - MEAN_RAW_H2) / STD_DEV_RAW_H2;
    features[4] = ((float)s->raw_ethanol - MEAN_RAW_ETHANOL) / STD_DEV_RAW_ETHANOL;
    features[5] = (s->pressure - MEAN_PRESSURE) / STD_DEV_PRESSURE;
    features[6] = ((float)s->pm1_0 - MEAN_PM1_0) / STD_DEV_PM1_0;
    features[7] = ((float)s->pm2_5 - MEAN_PM2_5) / STD_DEV_PM2_5;
    features[8] = ((float)s->nc0_5 - MEAN_NC0_5) / STD_DEV_NC0_5;
    
    /*for (int i = 0; i < 9; i++) {
        printf("features[%d] = %f\n", i, features[i]);
    }*/
    printf("NN model activation: %p\n", eml_net_activation_function_strs);
    // This is needed to avoid compiler error (warnings == errors)
    
    /* ----------- invoke predictor ----------- */
    bool is_fire = eml_net_predict(&fire_detector, features, 9) != 0;
    return is_fire;
}


PROCESS(smart_smoke_detector_process, "Smart Smoke Detector");
AUTOSTART_PROCESSES(&smart_smoke_detector_process);

PROCESS_THREAD(smart_smoke_detector_process, ev, data)
{
  PROCESS_BEGIN();
  
  /* ------ Sensor Resources Initialization ------ */
  LOG_INFO("Activating sensors\n");
  initialize_sensor_resources();
  
  /* --------- Activating CoAP Resource ---------- */
  LOG_INFO("Starting Smart Smoke Detector Server\n");
  activate_all_resources();
  
  /* --- Sensor Simulation: First Measurement ---- */
  LOG_INFO("Starting sensing environment\n");
  simulate_first_measurements(&sensors);
  update_sensor_resources(&sensors);

#ifdef COOJA
  leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
#else
  leds_on(LEDS_GREEN);
#endif

  /* --- Periodic Sensing Timer setting --- */
  etimer_set(&e_sensing_timer, CLOCK_SECOND * SENSORS_UPDATE_PERIOD);
  
  while(1) {
    PROCESS_WAIT_EVENT();
	
	/* ============= New Measurement Periodic Event ============= */
	if(ev == PROCESS_EVENT_TIMER && data == &e_sensing_timer){
	
		LOG_DBG("Sensors value update\n");
		simulate_new_measurements(&sensors,  simulate_fire_ignition, simulate_hazard_condition);
		update_sensor_resources(&sensors);
		
		
		unsigned int old_status = status;
		/* ------------ check scenario 1 (FIRE DETECTED - status 1) ------------*/
		//  activated if:
		//		- fast changes are detected (confront current vs 5th (or older avail.) measures)
		//		- status 0 OR 2 (if status=1 another timer will handle it)
		//		- positive output from AI model fire detection (probability > 0.5)
		if(fast_change_detected(&sensors, 5)) { 
			LOG_INFO("Fast Environment Change Detected\n");
			
			bool fire_detec = (bool)fire_detected(&sensors);
			/* ------ AI model fire detection ------ */
			if(fire_detec && (status != 1)){
				LOG_INFO("Fire Detected\n");
				/* ------- update status resource -------- */
				status = 1;
				/* ------ activate water sprinkler ------- */
				water_sprinkler = true;
				/* ----- Red Light Alarm Activation ------ */
				leds_off(LEDS_ALL);
#ifdef COOJA
				leds_on(LEDS_NUM_TO_MASK(LEDS_RED));
#else
				leds_on(LEDS_RED);
#endif
			} else if ((!fire_detec) && (status == 1)){
				/* --  -- */
				LOG_INFO("No fire detected\n");
				/* ------- update status resource -------- */
				status = 0;
				/* ----- deactivate water sprinkler ------ */
				water_sprinkler = false;
				/* ------ Light Alarm Deactivation ------- */
				leds_off(LEDS_ALL);
#ifdef COOJA
				leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
#else
				leds_on(LEDS_GREEN);
#endif
			}
		}
		/* ------------ check scenario 2 (HAZARD CONDITIONS DETECTED - status 2) ------------*/
		//  activated if:
		//		- any of PM parameters is above its safe limit
		//		- status 0 only (if status=1 Fire has higher priority than hazard conditions)
		if((status == 0) && above_safe_limits(&sensors)){
			LOG_INFO("Hazard conditions Detected\n");
			/* ------- update status resource -------- */
			status = 2;
			/* ---- Yellow Light Alarm Activation ---- */
			leds_off(LEDS_ALL);
#ifdef COOJA
			leds_on(LEDS_NUM_TO_MASK(LEDS_YELLOW));
#else
			leds_single_on(LEDS_YELLOW);
#endif
			/* ---- remote ventilation activation ---- */
			// trigger actuator device ------------------TO DO----------------------
			
		/* ------------ check scenario 0 (NORMAL CONDITIONS RESTORED - status 0) ------------*/
		//  activated if:
		//		- all PMs has fallen back within safe limits
		// 		- only if status 2 (Fire extinguished Assessment handled separatedly)
		} else if ((status == 2) && !above_safe_limits(&sensors)){
			LOG_INFO("Environment conditions back to normal\n");
			/* ------- update status resource -------- */
			status = 0;
			/* ------ Light Alarm Deactivation ------- */
			leds_off(LEDS_ALL);
#ifdef COOJA
			leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
#else
			leds_on(LEDS_GREEN);
#endif
		}
		
		/* -------- Status CoAP Resource Subscribers Notification -------- */
		// if status has changed, all subscribers to status resource are notified
		// actuators, acting as subscribers, are operated accordingly
		if(old_status != status){
			res_status.trigger();
		}
		
		/* -------- Sensor CoAP Resources Subscribers Periodic Notification -------- */
		// at the end of each buffer cycle ONLY
		if(is_temp_buffer_cycle_complete()) 	   {res_temp.trigger();}
		if(is_hum_buffer_cycle_complete()) 	   {res_hum.trigger();}
		if(is_pressure_buffer_cycle_complete())    {res_pressure.trigger();}
		if(is_tvoc_buffer_cycle_complete()) 	   {res_tvoc.trigger();}
		if(is_raw_h2_buffer_cycle_complete()) 	   {res_raw_h2.trigger();}
		if(is_raw_ethanol_buffer_cycle_complete()) {res_raw_ethanol.trigger();}
		if(is_pm1_0_buffer_cycle_complete()) 	   {res_pm1_0.trigger();}
		if(is_pm2_5_buffer_cycle_complete()) 	   {res_pm2_5.trigger();}
		if(is_nc0_5_buffer_cycle_complete()) 	   {res_nc0_5.trigger();}
		
		/* ------ Periodic Sensing Timer periodic setting ------ */
		etimer_set(&e_sensing_timer, CLOCK_SECOND * SENSORS_UPDATE_PERIOD);
	}
	
	/* ============ Button Release Event: Hazard/Fire Conditions Simulator ============ */
	else if(ev == button_hal_release_event) {
		btn = (button_hal_button_t *)data;
		if(btn->press_duration_seconds > BUTTON_PRESS_TIME_TO_FIRE) {
			simulate_fire_ignition = !simulate_fire_ignition;
			if(simulate_fire_ignition){
				LOG_DBG("FIRE SIMULATION START \n");
			} else {
				LOG_DBG("FIRE SIMULATION END \n");
			}
		} else {
			simulate_hazard_condition = !simulate_hazard_condition;
			if(simulate_hazard_condition){
				LOG_DBG("HAZARD SIMULATION START \n");
			} else {
				LOG_DBG("HAZARD SIMULATION END \n");
			}
		}
	}
  }
  
  PROCESS_END();
}


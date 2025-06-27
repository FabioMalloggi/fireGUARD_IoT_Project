#include "lib/ventilation_control.h"
#include "dev/leds.h"
#include "coap-engine.h"
#include "sys/log.h"

#define LOG_MODULE "SV-Vent-C"
#define LOG_LEVEL LOG_LEVEL_APP

/* ------ Ventilation Resources Inner Data Structure ------ */
extern int filter_vent;		// hazard ventilation system connection simulator
extern int smoke_vent;		// smoke control ventilation system connection simulator

/* ======= HAZARD VENTILATION: ACTIVATION REQUEST ======= */
static int filter_vent_on_req(coap_message_t *response){
  
  /* --- check filter vent already active ---- */
  if (filter_vent == MODE_ON){
    if(response) coap_set_status_code(response, VALID_2_03); // request was understood and valid, but it did not lead to a state change. A PUT or POST request is received that requests the same state the resource already has.
    return 0;
  }
  
  /* ----------- Actual Activation ----------- */
  else {
    smoke_vent = MODE_OFF;	// smoke ventilation deactivation
    filter_vent = MODE_ON;	// hazard ventilation activation
    leds_off(LEDS_ALL);
#ifdef COOJA
    leds_on(LEDS_NUM_TO_MASK(LEDS_YELLOW));	// proper light alarm activation
#else
    leds_single_on(LEDS_YELLOW);
#endif
    if(response) coap_set_status_code(response, CHANGED_2_04);
    return 1;
    }
}



/* ======= SMOKE VENTILATION: ACTIVATION REQUEST ======= */
static int smoke_vent_on_req(coap_message_t *response){
  
  /* --- check smoke vent already active ---- */
  if(smoke_vent == MODE_ON){
    if(response) coap_set_status_code(response, VALID_2_03);// request was understood and valid, but it did not lead to a state change. A PUT or POST request is received that requests the same state the resource already has.
    return 0;
  }
  
  /* ----------- Actual Activation ----------- */
  else {
    smoke_vent = MODE_ON;	// smoke ventilation activation
    filter_vent = MODE_OFF;	// hazard ventilation deactivation
    leds_off(LEDS_ALL);
#ifdef COOJA
    leds_on(LEDS_NUM_TO_MASK(LEDS_RED));	// proper light alarm activation
#else
    leds_on(LEDS_RED);
#endif
    if(response) coap_set_status_code(response, CHANGED_2_04);
    return 1;
  }
}

/* ======= HAZARD VENTILATION: DEACTIVATION REQUEST ======= */
static int filter_vent_off_req(coap_message_t *response){
  
  /* --- check filter vent already inactive ---- */
  if(filter_vent == MODE_OFF){
    if(response) coap_set_status_code(response, VALID_2_03);// request was understood and valid, but it did not lead to a state change. A PUT or POST request is received that requests the same state the resource already has.
    return 0;
  }
  
  /* ----------- Actual Deactivation ----------- */
  else {
    filter_vent = MODE_OFF;	// hazard ventilation deactivation
    leds_off(LEDS_ALL);
#ifdef COOJA
    leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
#else
    leds_on(LEDS_BLUE);
#endif
    if(response) coap_set_status_code(response, CHANGED_2_04);
    return 1;
  }
}



/* ======= SMOKE VENTILATION: DEACTIVATION REQUEST ======= */
static int smoke_vent_off_req(coap_message_t *response){
    
  /* --- check smoke vent already inactive ---- */
  if(smoke_vent == MODE_OFF){
    if(response) coap_set_status_code(response, VALID_2_03);// request was understood and valid, but it did not lead to a state change. A PUT or POST request is received that requests the same state the resource already has.
    return 0;
  }
  
  /* ----------- Actual Deactivation ----------- */
  else {
    smoke_vent = MODE_OFF;	// smoke ventilation deactivation
    leds_off(LEDS_ALL);
#ifdef COOJA
    leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
#else
    leds_on(LEDS_BLUE);
#endif
    if(response) coap_set_status_code(response, CHANGED_2_04);
    return 1;
  }
}



void handle_ventilation_request(int system_req, int mode_req, coap_message_t *response) {
  int return_code = -1;
  
  /* ======= HAZARD VENTILATION: ON REQUEST ======= */
  if(system_req == FILTER_VENT && mode_req == MODE_ON) {
    LOG_INFO("Activating FILTER vent..\n");
    return_code = filter_vent_on_req(response);
    
    if(return_code >= 1) 	{ LOG_DBG("FILTER Vent Activated\n");}
    else if (return_code == 0) 	{ LOG_DBG("FILTER Vent already Active\n"); }
    else 			{ LOG_DBG("FILTER Vent cannot be activated \n"); }
    
    
  /* ======= HAZARD VENTILATION: OFF REQUEST ======= */
  } else if(system_req == FILTER_VENT && mode_req == MODE_OFF) {
    LOG_INFO("Deactivating FILTER vent..\n");
    return_code = filter_vent_off_req(response);
    
    if(return_code >= 1) 	{ LOG_DBG("FILTER Vent Dectivated\n");}
    else if (return_code == 0) 	{ LOG_DBG("FILTER Vent already inactive\n"); }
  
  /* ======= SMOKE VENTILATION: ON REQUEST ======= */
  } else if(system_req == SMOKE_VENT && mode_req == MODE_ON) { 
    LOG_INFO("Activating SMOKE vent..\n"); 
    return_code = smoke_vent_on_req(response);
    
    if(return_code >= 1) 	{ LOG_DBG("SMOKE Vent Activated\n");}
    else if (return_code == 0) 	{ LOG_DBG("SMOKE Vent already Active\n"); }
    
    
  /* ======= SMOKE VENTILATION: OFF REQUEST ======= */
  } else if(system_req == SMOKE_VENT && mode_req == MODE_OFF) {
    LOG_INFO("Deactivating SMOKE vent..\n");
    return_code = smoke_vent_off_req(response);
    
    if(return_code >= 1) 	{ LOG_DBG("SMOKE Vent Dectivated\n");}
    else if (return_code == 0) 	{ LOG_DBG("SMOKE Vent already inactive\n"); }
  }
}

void set_vent_by_status(int status_value){
  if(status_value == HAZARD_STATUS) {
    handle_ventilation_request(FILTER_VENT, MODE_ON, NULL);      
    
  } else if(status_value == FIRE_STATUS) {
    handle_ventilation_request(SMOKE_VENT, MODE_ON, NULL);
         
  } else if(status_value == NORMAL_STATUS) {
    handle_ventilation_request(SMOKE_VENT, MODE_OFF, NULL);
    handle_ventilation_request(FILTER_VENT, MODE_OFF, NULL);
  }
}


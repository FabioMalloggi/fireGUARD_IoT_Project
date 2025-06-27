#ifndef SENSOR_SIM_H
#define SENSOR_SIM_H

#include <stdbool.h>

// Sensor Data Structure
typedef struct {
    float temperature;
    float humidity;
    float pressure;
    int tvoc;
    int raw_h2;
    int raw_ethanol;
    int pm1_0;
    int pm2_5;
    int nc0_5;
    int fire_recovery_count;
    int hazard_recovery_count;
} sensor_sim_t;


#define SENSORS_UPDATE_PERIOD  3 //sec

// Constants


#define TEMP_MIN         -22.0f
#define TEMP_STD          15.0f
#define TEMP_MAX          60.0f
#define TEMP_FIRE         38.2f
#define TEMP_FIRE_STEP ((TEMP_FIRE - TEMP_STD) / 5.0f)  // = (19.5 - 15)/5 = 0.9f
#define TEMP_DELTA     (TEMP_FIRE_STEP / 20) 		// = (0.9)/20 = 0.045f

#define HUMIDITY_MIN      10.7f
#define HUMIDITY_STD      25.0f
#define HUMIDITY_MAX      75.0f
#define HUMIDITY_FIRE     15.5f
#define HUMIDITY_FIRE_STEP ((HUMIDITY_STD - HUMIDITY_FIRE) / 5.0f)  // = (25 - 15.5)/5 = 1.9f
#define HUMIDITY_DELTA     (HUMIDITY_FIRE_STEP / 20) 		    // = (1.9)/10 = 0.095f

#define PRESSURE_MIN     930.0f
#define PRESSURE_STD     936.9f
#define PRESSURE_MAX     940.0f
#define PRESSURE_FIRE    930.9f
#define PRESSURE_FIRE_STEP ((PRESSURE_STD - PRESSURE_FIRE) / 5.0f)  // = (936.9 - 930.9)/5 = 1.2f
#define PRESSURE_DELTA     (PRESSURE_FIRE_STEP / 20) 		    // = (1.2)/20 = 0.06f

#define TVOC_MIN            0
#define TVOC_STD        38000
#define TVOC_MAX        60000
#define TVOC_FIRE           0
#define TVOC_FIRE_STEP ((TVOC_STD - TVOC_FIRE) / 5)  	// = (38000 - 0)/5 = 7600
#define TVOC_DELTA     (TVOC_FIRE_STEP / 20) 		// = (7600)/20 = 380

#define RAW_H2_MIN      10700
#define RAW_H2_STD      11300
#define RAW_H2_MAX      13800
#define RAW_H2_FIRE     12700
#define RAW_H2_FIRE_STEP ((RAW_H2_FIRE - RAW_H2_STD) / 5)  // = (12700 - 11300)/5 = 280
#define RAW_H2_DELTA     (RAW_H2_FIRE_STEP / 20) 	   // = (280)/20 = 14

#define RAW_ETH_MIN     15300
#define RAW_ETH_STD     16500
#define RAW_ETH_MAX     21400
#define RAW_ETH_FIRE	20500
#define RAW_ETH_FIRE_STEP ((RAW_ETH_FIRE - RAW_ETH_STD) / 5)  // = (20500 - 16500)/5 = 800
#define RAW_ETH_DELTA     (RAW_ETH_FIRE_STEP / 20) 	      // = (800)/20 = 40

#define PM1_0_MIN           0
#define PM1_0_STD           0
#define PM1_0_MAX        1000
#define PM1_0_HAZARD      500
#define PM1_0_HAZARD_STEP ((PM1_0_HAZARD - PM1_0_STD) / 5)  // = (500)/5 = 100
#define PM1_0_DELTA       (PM1_0_HAZARD_STEP / 20) 	    // = (100)/20 = 5

#define PM2_5_MIN           0
#define PM2_5_STD           0
#define PM2_5_MAX        1000
#define PM2_5_HAZARD      500
#define PM2_5_HAZARD_STEP ((PM2_5_HAZARD - PM2_5_STD) / 5)  // = (500)/5 = 100
#define PM2_5_DELTA       (PM2_5_HAZARD_STEP / 20) 	    // = (100)/20 = 5

#define NC0_5_MIN           0
#define NC0_5_STD           0
#define NC0_5_MAX       20000
#define NC0_5_HAZARD    10000
#define NC0_5_HAZARD_STEP ((NC0_5_HAZARD - NC0_5_STD) / 5)  // = (10000)/5 = 2000
#define NC0_5_DELTA       (NC0_5_HAZARD_STEP / 20) 	    // = (2000)/20 = 100


#define MAX_FIRE_RECOVERY_PERIOD    5 //measurements
#define MAX_HAZARD_RECOVERY_PERIOD  5 //measurements

// Functions
void simulate_first_measurements(sensor_sim_t *s);
void simulate_new_measurements(sensor_sim_t *s, bool is_fire, bool is_hazard);

#endif // SENSOR_SIM_H

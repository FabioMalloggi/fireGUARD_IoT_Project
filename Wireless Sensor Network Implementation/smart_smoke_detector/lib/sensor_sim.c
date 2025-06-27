#include "sensor_sim.h"
#include <stdlib.h> // for abs()
#include <math.h> // for fabsf()

static float clamp(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}
static int clamp_int(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static float random_delta(float max_delta) {
    float delta = ((float)rand() / RAND_MAX) * max_delta;
    return (rand() % 2 == 0) ? delta : -delta;
}

static int random_delta_int(int max_delta) {
    int delta = rand() % (max_delta + 1); //rand() returns pseudorandom values much larger than 100 000
    return (rand() % 2 == 0) ? delta : -delta;
}

static float step_towards(float current, float target, float step) {
    if (fabsf(current - target) <= step) return target;
    return current + (current < target ? step : -step);
}

static int step_towards_int(int current, int target, int step) {
    if (abs(current - target) <= step) return target;
    return current + (current < target ? step : -step);
}

void simulate_first_measurements(sensor_sim_t *s) {
    if (!s) return;
    srand(12345);

    s->temperature   = TEMP_STD;
    s->humidity      = HUMIDITY_STD;
    s->pressure      = PRESSURE_STD;
    s->tvoc          = TVOC_STD;
    s->raw_h2        = RAW_H2_STD;
    s->raw_ethanol   = RAW_ETH_STD;
    s->pm1_0         = PM1_0_STD;
    s->pm2_5         = PM2_5_STD;
    s->nc0_5         = NC0_5_STD;
    s->hazard_recovery_count = 0;
    s->fire_recovery_count = 0;
}




void simulate_new_measurements(sensor_sim_t *s, bool is_fire, bool is_hazard) {
    if (!s) return;

    if (is_fire) {
	s->temperature = step_towards(s->temperature, TEMP_FIRE,     TEMP_FIRE_STEP);
	s->humidity    = step_towards(s->humidity,    HUMIDITY_FIRE, HUMIDITY_FIRE_STEP);
	s->pressure    = step_towards(s->pressure,    PRESSURE_FIRE, PRESSURE_FIRE_STEP);
	s->tvoc        = step_towards_int(s->tvoc,        TVOC_FIRE,     TVOC_FIRE_STEP);
	s->raw_h2      = step_towards_int(s->raw_h2,      RAW_H2_FIRE,   RAW_H2_FIRE_STEP);
	s->raw_ethanol = step_towards_int(s->raw_ethanol, RAW_ETH_FIRE,  RAW_ETH_FIRE_STEP);

        if (s->fire_recovery_count < MAX_FIRE_RECOVERY_PERIOD) {
            s->fire_recovery_count++;
        }
	
    } else if (s->fire_recovery_count > 0){
        s->temperature = step_towards(s->temperature, TEMP_STD,     TEMP_FIRE_STEP);
        s->humidity    = step_towards(s->humidity,    HUMIDITY_STD, HUMIDITY_FIRE_STEP);
        s->pressure    = step_towards(s->pressure,    PRESSURE_STD, PRESSURE_FIRE_STEP);
        s->tvoc        = step_towards_int(s->tvoc,        TVOC_STD,     TVOC_FIRE_STEP);
        s->raw_h2      = step_towards_int(s->raw_h2,      RAW_H2_STD,   RAW_H2_FIRE_STEP);
        s->raw_ethanol = step_towards_int(s->raw_ethanol, RAW_ETH_STD,  RAW_ETH_FIRE_STEP);

        s->fire_recovery_count--;
    } else {
	s->temperature = clamp(s->temperature + random_delta(TEMP_DELTA),        TEMP_MIN,     TEMP_MAX);
	s->humidity    = clamp(s->humidity    + random_delta(HUMIDITY_DELTA),    HUMIDITY_MIN, HUMIDITY_MAX);
	s->pressure    = clamp(s->pressure    + random_delta(PRESSURE_DELTA),    PRESSURE_MIN, PRESSURE_MAX);
	s->tvoc        = clamp_int(s->tvoc        + random_delta_int(TVOC_DELTA),    TVOC_MIN,     TVOC_MAX);
	s->raw_h2      = clamp_int(s->raw_h2      + random_delta_int(RAW_H2_DELTA),  RAW_H2_MIN,   RAW_H2_MAX);
	s->raw_ethanol = clamp_int(s->raw_ethanol + random_delta_int(RAW_ETH_DELTA), RAW_ETH_MIN,  RAW_ETH_MAX);
    }
    
    
    if(is_hazard){
    	s->pm1_0       = step_towards_int(s->pm1_0,       PM1_0_HAZARD,    PM1_0_HAZARD_STEP);
	s->pm2_5       = step_towards_int(s->pm2_5,       PM2_5_HAZARD,    PM2_5_HAZARD_STEP);
	s->nc0_5       = step_towards_int(s->nc0_5,       NC0_5_HAZARD,    NC0_5_HAZARD_STEP);
	
	if (s->hazard_recovery_count < MAX_HAZARD_RECOVERY_PERIOD) {
            s->hazard_recovery_count++;
        }
    } else if (s->hazard_recovery_count > 0){
    	s->pm1_0       = step_towards_int(s->pm1_0,       PM1_0_STD,    PM1_0_HAZARD_STEP);
	s->pm2_5       = step_towards_int(s->pm2_5,       PM2_5_STD,    PM2_5_HAZARD_STEP);
	s->nc0_5       = step_towards_int(s->nc0_5,       NC0_5_STD,    NC0_5_HAZARD_STEP);
	
	s-> hazard_recovery_count--;
    } else {
    	s->pm1_0       = clamp_int(s->pm1_0       + random_delta_int(PM1_0_DELTA),   PM1_0_MIN,    PM1_0_MAX);
        s->pm2_5       = clamp_int(s->pm2_5       + random_delta_int(PM2_5_DELTA),   PM2_5_MIN,    PM2_5_MAX);
        s->nc0_5       = clamp_int(s->nc0_5       + random_delta_int(NC0_5_DELTA),   NC0_5_MIN,    NC0_5_MAX);
    }
}






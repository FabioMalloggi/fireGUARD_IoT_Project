#include "lib/smart_smoke_detector_utilities.h"
#include <math.h>     //for fabsf()
#include <stdlib.h>   //for abs()

void initialize_sensor_resources(void){
  init_measurements_series(&temp_series, "temp", "C", SENML_FLOAT);
  init_measurements_series(&hum_series,  "hum",  "%", SENML_FLOAT);
  init_measurements_series(&pressure_series,  "pressure",  "hPa", SENML_FLOAT);
  init_measurements_series(&tvoc_series,  "tvoc",  "ppb", SENML_INT);
  init_measurements_series(&raw_h2_series,  "raw_h2",  "ppm", SENML_INT);
  init_measurements_series(&raw_ethanol_series,  "raw_ethanol",  "ppm", SENML_INT);
  init_measurements_series(&pm1_0_series,  "pm1_0",  "µg/m3", SENML_INT);
  init_measurements_series(&pm2_5_series,  "pm2_5",  "µg/m3", SENML_INT);
  init_measurements_series(&nc0_5_series,  "nc0_5",  "p/cm3", SENML_INT);
}

void update_sensor_resources(const sensor_sim_t *sensors) {
  add_measurement(&temp_series, sensors->temperature);
  add_measurement(&hum_series,  sensors->humidity);
  add_measurement(&pressure_series,  sensors->pressure);
  add_measurement_int(&tvoc_series,  sensors->tvoc);
  add_measurement_int(&raw_h2_series,  sensors->raw_h2);
  add_measurement_int(&raw_ethanol_series,  sensors->raw_ethanol);
  add_measurement_int(&pm1_0_series,  sensors->pm1_0);
  add_measurement_int(&pm2_5_series,  sensors->pm2_5);
  add_measurement_int(&nc0_5_series,  sensors->nc0_5);
}

// return true if any of the last sensed parameter is changing too rapidly:
// if any newly sensed parameter differs more than two times its HAZARD_STEP with respect to its last nth sensed measurements (older one if not enough elements are stored)
bool fast_change_detected(const sensor_sim_t *sensors, int nth_elem) {
  return
    fabsf(get_nth_last_float(&temp_series, nth_elem)     - sensors->temperature) > (TEMP_FIRE_STEP * 2.0f) ||
    fabsf(get_nth_last_float(&hum_series, nth_elem)      - sensors->humidity)    > (HUMIDITY_FIRE_STEP * 2.0f) ||
    fabsf(get_nth_last_float(&pressure_series, nth_elem) - sensors->pressure)    > (PRESSURE_FIRE_STEP * 2.0f) ||
    abs(get_nth_last_int(&tvoc_series, nth_elem)         - sensors->tvoc)        > (TVOC_FIRE_STEP * 2) ||
    abs(get_nth_last_int(&raw_h2_series, nth_elem)       - sensors->raw_h2)      > (RAW_H2_FIRE_STEP * 2) ||
    abs(get_nth_last_int(&raw_ethanol_series, nth_elem)  - sensors->raw_ethanol) > (RAW_ETH_FIRE_STEP * 2);
}

bool is_temp_buffer_cycle_complete(void){        return is_buffer_cycle_complete(&temp_series); }
bool is_hum_buffer_cycle_complete(void){         return is_buffer_cycle_complete(&hum_series); }
bool is_pressure_buffer_cycle_complete(void){    return is_buffer_cycle_complete(&pressure_series); }
bool is_tvoc_buffer_cycle_complete(void){        return is_buffer_cycle_complete(&tvoc_series); }
bool is_raw_h2_buffer_cycle_complete(void){      return is_buffer_cycle_complete(&raw_h2_series); }
bool is_raw_ethanol_buffer_cycle_complete(void){ return is_buffer_cycle_complete(&raw_ethanol_series); }
bool is_pm1_0_buffer_cycle_complete(void){       return is_buffer_cycle_complete(&pm1_0_series); }
bool is_pm2_5_buffer_cycle_complete(void){       return is_buffer_cycle_complete(&pm2_5_series); }
bool is_nc0_5_buffer_cycle_complete(void){       return is_buffer_cycle_complete(&nc0_5_series); }


bool above_safe_limits(const sensor_sim_t *sensors){
  return
    sensors->pm1_0 >  pm1_0_limit||
    sensors->pm2_5 >  pm2_5_limit||
    sensors->nc0_5 >  nc0_5_limit;
}










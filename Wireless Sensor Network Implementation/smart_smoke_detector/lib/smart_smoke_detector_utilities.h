#include <stdbool.h>
#include "lib/senml_series.h"
#include "lib/sensor_sim.h"

// Resource Data Structures: Time series buffers organized as a senML object and transmitted as senML json object
extern senml_series temp_series;
extern senml_series hum_series;
extern senml_series pressure_series;
extern senml_series tvoc_series;
extern senml_series raw_h2_series;
extern senml_series raw_ethanol_series;
extern senml_series pm1_0_series;
extern senml_series pm2_5_series;
extern senml_series nc0_5_series;

extern int pm1_0_limit;
extern int pm2_5_limit;
extern int nc0_5_limit;

void initialize_sensor_resources(void);

void update_sensor_resources(const sensor_sim_t *sensors);

bool fast_change_detected(const sensor_sim_t *sensors, int nth_elem);

bool is_temp_buffer_cycle_complete(void);
bool is_hum_buffer_cycle_complete(void);
bool is_pressure_buffer_cycle_complete(void);
bool is_tvoc_buffer_cycle_complete(void);
bool is_raw_h2_buffer_cycle_complete(void);
bool is_raw_ethanol_buffer_cycle_complete(void);
bool is_pm1_0_buffer_cycle_complete(void);
bool is_pm2_5_buffer_cycle_complete(void);
bool is_nc0_5_buffer_cycle_complete(void);

bool above_safe_limits(const sensor_sim_t *sensors);



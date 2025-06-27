#include "lib/senml_series.h"
#include <stdio.h> // for snprintf
#include "os/sys/clock.h"
#include <locale.h>

void init_measurements_series(senml_series *series, const char *name, const char *unit, senml_value_type type) {
    snprintf(series->name, NAME_MAX_LEN, "%s", name);
    snprintf(series->unit, UNIT_MAX_LEN, "%s", unit);

    series->value_type = type;
    series->count = 0;
    series->index = 0;
}

void add_measurement(senml_series *series, float value) {
    if (series->value_type != SENML_FLOAT) return;

    unsigned long now = clock_seconds();
    series->records[series->index].fvalue = value;
    series->records[series->index].time = now;
    series->index = (series->index + 1) % HISTORY_SIZE;
    series->count++;
}

// integer version for type-safe programming
void add_measurement_int(senml_series *series, int value) {
    if (series->value_type != SENML_INT) return;

    unsigned long now = clock_seconds();
    series->records[series->index].ivalue = value;
    series->records[series->index].time = now;
    series->index = (series->index + 1) % HISTORY_SIZE;
    series->count++;
}

bool is_buffer_cycle_complete(senml_series *series){
    return (series->count % HISTORY_SIZE == 0);
}

void create_senml_json(const senml_series *series, char *payload, unsigned int payload_size, int req_m) { //size_t = unsigned long
    if (!series || series->count == 0) {
        snprintf(payload, payload_size, "{\"bn\":\"%s\",\"ver\":%d,\"e\":[]}", BASE_NAME, VERSION);
        return; // No measurements available yet
    }

    // Determine how many measurements are available
    int available_m = (series->count < HISTORY_SIZE) ? series->count : HISTORY_SIZE;

    // Decide how many measurements to include in the JSON
    int actual_m = (req_m <= 0 || req_m >= available_m) ? available_m : req_m;

    // Index of the newest record (most recent)
    int newest_index = (series->index - 1 + HISTORY_SIZE) % HISTORY_SIZE;

    /* Determine index of the oldest record to include, handling a circual buffer
    * (actual_m-1): remember that one measurement (newest) is already included
    * (actual_m-1) = -actual_m +1
    * if actual_m == HISTORY_SIZE: 
    *                (newest_index + 1 - HISTORY_SIZE + HISTORY_SIZE) % HISTORY_SIZE
    *              = (newest_index + 1) % HISTORY_SIZE
    *              = series->index                     // the next to be overwritten is oldest
    */
    // Index of the oldest record to include
    int oldest_index = (newest_index - actual_m + 1 + HISTORY_SIZE) % HISTORY_SIZE;

    // Use the timestamp of the oldest record as base time
    unsigned long base_time = series->records[oldest_index].time;

    // Begin the JSON payload
    int written = snprintf(payload, payload_size,
        "{\"bn\":\"%s%s\",\"bu\":\"%s\",\"ver\":%d,\"bt\":%lu,\"e\":[",
        BASE_NAME, series->name, series->unit, VERSION, base_time);

    setlocale(LC_NUMERIC, "C"); // guarantee snprintf %.3f as 123.456
    
    // Write each measurement as a SenML entry
    for (int i = 0; i < actual_m && written < payload_size; i++) {
        int idx = (oldest_index + i) % HISTORY_SIZE;
        int rel_time = (int)(series->records[idx].time - base_time);

        // Add comma separator for all but the first entry
        if (i > 0) {
            written += snprintf(payload + written, payload_size - written, ",");
        }

        // Add the value entry depending on type
        if (series->value_type == SENML_FLOAT) {
            int value_int = (int)(series->records[idx].fvalue * 100.0f);
            written += snprintf(payload + written, payload_size - written,
            "{\"v\":%d,\"t\":%d}", value_int, rel_time);
        } else if (series->value_type == SENML_INT) {
            written += snprintf(payload + written, payload_size - written,
                "{\"v\":%d,\"t\":%d}", series->records[idx].ivalue, rel_time);
        } else {
            // Unsupported value type
            snprintf(payload + written, payload_size - written, "]}");
            return;
        }
    }

    // Close the JSON object
    snprintf(payload + written, payload_size - written, "]}");
}


float get_nth_last_float(const senml_series *series, int requested_n) {
    if (!series || series->value_type != SENML_FLOAT || series->count == 0) {
        return -1.0f; // error or no element
    }
    /* available_n: 
    *        whatever the requested number, the buffer keeps the last HISTORY_SIZE measures at most
    *  receivable_n:
    *       if requested_n is smaller than 1 or higher than available, last inserted measurements is given 
    */
    int available_n = series->count < HISTORY_SIZE ? series->count : HISTORY_SIZE;
    int valid_requested_n = (requested_n <= 0 || requested_n > available_n) ? available_n : requested_n;
    
    int newest_index = (series->index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    int target_index = (newest_index - (valid_requested_n - 1) + HISTORY_SIZE) % HISTORY_SIZE;
    return series->records[target_index].fvalue;
}

// integer version for type-safe programming
int get_nth_last_int(const senml_series *series, int requested_n) {
    if (!series || series->value_type != SENML_INT || series->count == 0) {
        return -1; // error or no element
    }
    /* available_n: 
    *        whatever the requested number, the buffer keeps the last HISTORY_SIZE measures at most
    *  receivable_n:
    *       if requested_n is smaller than 1 or higher than available, last inserted measurements is given 
    */
    int available_n = series->count < HISTORY_SIZE ? series->count : HISTORY_SIZE;
    int valid_requested_n = (requested_n <= 0 || requested_n > available_n) ? available_n : requested_n;

    int newest_index = (series->index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    int target_index = (newest_index - (valid_requested_n - 1) + HISTORY_SIZE) % HISTORY_SIZE;

    return series->records[target_index].ivalue;
}





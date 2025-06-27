#ifndef SENML_SERIES_H
#define SENML_SERIES_H

#include "network_config.h" //for GLOBAL CONSTANTS
#include <stdbool.h> // for bool

// size of circular buffer
#define HISTORY_SIZE PAYLOAD_MAX_MEASUREMENTS

// senML json constants
#define BASE_NAME LOCAL_HOST  // fixed shared base prefix for all sensors
#define VERSION 1
#define NAME_MAX_LEN 16 //suffix name max length
#define UNIT_MAX_LEN 8

typedef enum {
    SENML_FLOAT,
    SENML_INT
} senml_value_type;

typedef struct {
    union {
        float fvalue;
        int ivalue;
    };
    unsigned long time; // seconds since epoch
} senml_record;

typedef struct {
    char name[NAME_MAX_LEN];
    char unit[UNIT_MAX_LEN];
    senml_value_type value_type;
    
    senml_record records[HISTORY_SIZE];
    int count;
    int index; // circular buffer index
} senml_series;


void init_measurements_series(senml_series *series, const char *name, const char *unit, senml_value_type type);

void add_measurement(senml_series *series, float value);
void add_measurement_int(senml_series *series, int value);

bool is_buffer_cycle_complete(senml_series *series);

void create_senml_json(const senml_series *series, char *buffer, unsigned int buf_size, int req_m);

float get_nth_last_float(const senml_series *series, int requested_n);
int get_nth_last_int(const senml_series *series, int requested_n);

#endif // SENML_SERIES_H


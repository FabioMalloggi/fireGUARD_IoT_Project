#ifndef VENTILATION_CONTROL_H
#define VENTILATION_CONTROL_H

#include "contiki.h"
#include "coap-engine.h"

// Constants
#define HAZARD_STATUS 2
#define FIRE_STATUS 1
#define NORMAL_STATUS 0

#define SMOKE_VENT FIRE_STATUS 		// 1
#define FILTER_VENT HAZARD_STATUS	// 2

#define MODE_ON 1
#define MODE_OFF 0

void handle_ventilation_request(int system_req, int mode_req, coap_message_t *response);

void set_vent_by_status(int status_value);

#endif // VENTILATION_CONTROL_H


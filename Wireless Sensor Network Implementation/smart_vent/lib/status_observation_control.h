#ifndef OBSERVATION_CONTROL_H_
#define OBSERVATION_CONTROL_H_

#include "coap-engine.h"

#define MODE_ON 1
#define MODE_OFF 0

int observation_init(const char *server_ep_str);

void toggle_observation(void);

int is_observing(void);

#endif /* OBSERVATION_CONTROL_H_ */

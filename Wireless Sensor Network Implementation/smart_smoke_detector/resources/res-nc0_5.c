#include "string.h" // for memcopy
#include <stdlib.h> // for atoi
#include "contiki.h"
#include "coap-engine.h"
#include "lib/sensor_sim.h"
#include "lib/senml_series.h"

// WARNING: 5000 particles/cm³ is a 1-sec severe pollution spike (Typical indoor safe level: <2000 particles/cm³)
#define NC0_5_STD_SAFE_LIMIT 2500

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

// Custom-compliant resource type(rt); ct=50: CoAP content-format number for SenML JSON encoding
EVENT_RESOURCE(res_nc0_5,
		  "title=\"NC0.5: ?n=0, POST/PUT limit=<limit>\";rt=\"urn:ietf:senml:json:nc0_5\";ct=50;obs", 
		  res_get_handler,
		  res_post_put_handler,
		  res_post_put_handler,
		  NULL,
		  res_event_handler);


senml_series nc0_5_series; // Time series buffer organized as a senML object and transmitted as senML json object

int nc0_5_limit = NC0_5_STD_SAFE_LIMIT;

static void res_event_handler(void) {
  coap_notify_observers(&res_nc0_5);
}


static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  char payload[MAX_PAYLOAD_LEN];
  
  const char *n = NULL;
  int n_measurements = -1;
  if(coap_get_query_variable(request, "n", &n)) {
    n_measurements = atoi(n); 
  }
  create_senml_json(&nc0_5_series, payload, sizeof(payload), n_measurements);

  int len = strlen(payload);
  memcpy(buffer, payload, len);

  coap_set_header_content_format(response, APPLICATION_JSON);
  coap_set_payload(response, buffer, len);
}

static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  const char *text = NULL;
  int limit = NC0_5_STD_SAFE_LIMIT;
  if(coap_get_post_variable(request, "limit", &text)){
    limit = atoi(text);
    if(limit < 0) { limit = 0; }
    else if (limit > NC0_5_MAX) { limit = NC0_5_MAX; }
    
    nc0_5_limit = limit;
    coap_set_status_code(response, CHANGED_2_04);
  } else { 
    coap_set_status_code(response, BAD_REQUEST_4_00); }
}


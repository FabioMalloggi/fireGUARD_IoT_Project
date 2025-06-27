#include "string.h" // for memcopy
#include <stdlib.h> // for atoi
#include "contiki.h"
#include "coap-engine.h"
#include "lib/sensor_sim.h"
#include "lib/senml_series.h"

// WARNING: 500 µg/m³ is a 1-sec hazardous spike (WHO annual safe limit: 10 µg/m³)
#define PM1_0_STD_SAFE_LIMIT 250

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

// Custom-compliant resource type(rt); ct=50: CoAP content-format number for SenML JSON encoding
EVENT_RESOURCE(res_pm1_0,
		  "title=\"PM1.0: ?n=0, POST/PUT limit=<limit>\";rt=\"urn:ietf:senml:json:pm1_0\";ct=50;obs", 
		  res_get_handler,
		  res_post_put_handler,
		  res_post_put_handler,
		  NULL,
		  res_event_handler);
  

senml_series pm1_0_series; // Time series buffer organized as a senML object and transmitted as senML json object

int pm1_0_limit = PM1_0_STD_SAFE_LIMIT; // peak limit for safety conditions

static void res_event_handler(void) {
  coap_notify_observers(&res_pm1_0);
}


static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  char payload[MAX_PAYLOAD_LEN];
  
  const char *n = NULL;
  int n_measurements = -1;
  if(coap_get_query_variable(request, "n", &n)) {
    n_measurements = atoi(n); 
  }
  create_senml_json(&pm1_0_series, payload, sizeof(payload), n_measurements);

  int len = strlen(payload);
  memcpy(buffer, payload, len);

  coap_set_header_content_format(response, APPLICATION_JSON);
  coap_set_payload(response, buffer, len);
}

static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  const char *text = NULL;
  int limit = PM1_0_STD_SAFE_LIMIT;
  if(coap_get_post_variable(request, "limit", &text)){
    limit = atoi(text);
    if(limit < 0) { limit = 0; }
    else if (limit > PM1_0_MAX) { limit = PM1_0_MAX; }
    
    pm1_0_limit = limit;
    coap_set_status_code(response, CHANGED_2_04);
  } else { 
    coap_set_status_code(response, BAD_REQUEST_4_00); }
}




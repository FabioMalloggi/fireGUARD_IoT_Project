#include "string.h" // for memcopy
#include <stdlib.h> // for atoi
#include "contiki.h"
#include "coap-engine.h"
#include "lib/senml_series.h"

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);


EVENT_RESOURCE(res_tvoc,
		  "title=\"TVOC: ?n=0..\";rt=\"urn:ietf:senml:json:tvoc\";ct=50;obs", // Custom-compliant resource type(rt); ct=50: CoAP content-format number for SenML JSON encoding
		  res_get_handler,
		  NULL,
		  NULL,
		  NULL,
		  res_event_handler);
  

senml_series tvoc_series; // Time series buffer organized as a senML object and transmitted as senML json object


static void res_event_handler(void) {
  coap_notify_observers(&res_tvoc);
}


static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  char payload[MAX_PAYLOAD_LEN];
  
  const char *n = NULL;
  int n_measurements = -1;
  if(coap_get_query_variable(request, "n", &n)) {
    n_measurements = atoi(n); 
  }
  create_senml_json(&tvoc_series, payload, sizeof(payload), n_measurements);

  int len = strlen(payload);
  memcpy(buffer, payload, len);

  coap_set_header_content_format(response, APPLICATION_JSON);
  coap_set_payload(response, buffer, len);
}





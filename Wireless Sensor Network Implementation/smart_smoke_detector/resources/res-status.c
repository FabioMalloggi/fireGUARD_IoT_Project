#include "string.h" // for memcopy
#include <stdio.h> // for snprintf
#include "contiki.h"
#include "coap-engine.h"
#include "lib/senml_series.h"

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);


EVENT_RESOURCE(res_status,
		  "title=\"Status\";rt=\"urn:ietf:senml:json:status\";obs",
		  res_get_handler,
		  NULL,
		  NULL,
		  NULL,
		  res_event_handler);
  

unsigned int status = 0;

static void res_event_handler(void) {
  coap_notify_observers(&res_status);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  char payload[MAX_PAYLOAD_LEN];

  int len = snprintf(payload, sizeof(payload),
                     "{\"bn\": \"%sstatus\", \"status\": %u, \"bt\": %lu}", 
                     BASE_NAME, status, (unsigned long)clock_seconds());

  memcpy(buffer, payload, len);
  coap_set_header_content_format(response, APPLICATION_JSON);
  coap_set_payload(response, buffer, len);
}





#include "contiki.h"
#include "coap-engine.h"
#include "lib/ventilation_control.h"
#include <string.h>
#include "os/sys/clock.h"
#include "lib/network_config.h" // for senML constants

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "res_vent"
#define LOG_LEVEL LOG_LEVEL_APP

static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple actuator example, depending on the color query parameter and post variable mode, corresponding led is activated or deactivated */
RESOURCE(res_vent,
		 "title=\"Vent: ?system=filter|smoke, POST/PUT mode=on|off\";rt=\"Control\"",
		 NULL,
		 res_post_put_handler,
		 res_post_put_handler,
		 NULL);

int filter_vent = MODE_OFF;
int smoke_vent = MODE_OFF;



static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *system = NULL;
  const char *mode = NULL;
  
  int system_req = -1; // smoke_system = 1 ; filter_system = 2;
  int mode_req = -1; // mode_on = 1 ; mode_off = 0;
  
  LOG_DBG("Received Post/Put Request\n");
  
  // Retrieve first query variable
  if((len = coap_get_query_variable(request, "system", &system))) {
    LOG_DBG("requested sistem: %.*s\n", (int)len, system);

    if(strncmp(system, "filter", len) == 0) {
      system_req = FILTER_VENT;
    } else if(
      strncmp(system, "smoke", len) == 0) {
      system_req = SMOKE_VENT;
    }
  }
  
  // Retrieve second query variable
  if((system_req > 0) && (len = coap_get_post_variable(request, "mode", &mode))) {
    LOG_DBG("requested mode: %s\n", mode);

    if(strncmp(mode, "on", len) == 0) {
      mode_req = MODE_ON;
    } else if(
      strncmp(mode, "off", len) == 0) {
      mode_req = MODE_OFF;
    } 
  }
  
  /* ====== UNSUCCESSFUL MESSAGE RETRIEVE ====== */
  // at least one query variable was not received correctly
  if(mode_req < 0) {
    coap_set_status_code(response, BAD_REQUEST_4_00);
  }
  
  /* ======= SUCCESSFUL MESSAGE RETRIEVE ======= */
  // both mode_req and system_req have meaningful values
  else {
    // ventilation_control.h handler
    handle_ventilation_request(system_req, mode_req, response);
  }
}

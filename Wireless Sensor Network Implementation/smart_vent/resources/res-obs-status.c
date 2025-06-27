#include "contiki.h"
#include "coap-engine.h"
#include "lib/status_observation_control.h"
#include <string.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "res_obs"
#define LOG_LEVEL LOG_LEVEL_APP


static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple actuator example, depending on the color query parameter and post variable mode, corresponding led is activated or deactivated */
RESOURCE(res_obs_status,
         "title=\"obs_status: POST/PUT mode=on|off\";rt=\"Control\"",
         NULL,
         res_post_put_handler,
         res_post_put_handler,
         NULL);



static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *mode = NULL;

  int mode_req = -1; // mode_on = 1 ; mode_off = 0;
  
  LOG_DBG("Received Post/Put Request\n");
    
  // Retrieve first query variable
  if((len = coap_get_post_variable(request, "mode", &mode))) {
    LOG_DBG("requested mode: %s\n", mode);

    if(strncmp(mode, "on", len) == 0) {
      mode_req = MODE_ON;
    } else if(
      strncmp(mode, "off", len) == 0) {
      mode_req = MODE_OFF;
    }
  }
  
  /* ====== UNSUCCESSFUL MESSAGE RETRIEVE ====== */
  //query variable was not received correctly
  if(mode_req < 0) {
    coap_set_status_code(response, BAD_REQUEST_4_00);
  }
  
  /* ======= SUCCESSFUL MESSAGE RETRIEVE ======= */
  // mode_req has meaningful value
  else {
    if( ((mode_req == MODE_ON) && !(is_observing())) ||
        ((mode_req == MODE_OFF) && (is_observing()))    ){
      toggle_observation();
      coap_set_status_code(response, CHANGED_2_04);
    } else {
    coap_set_status_code(response, VALID_2_03);
    }
  }
}

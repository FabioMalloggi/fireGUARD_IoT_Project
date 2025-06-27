// reference: https://github.com/contiki-ng/contiki-ng/blob/develop/examples/coap/coap-example-client/coap-example-observe-client.c
#include "lib/status_observation_control.h"
#include "lib/ventilation_control.h"
#include "coap-engine.h"
#include "coap.h"
#include "sys/log.h"
#include "dev/leds.h"

#define LOG_MODULE "SV-Obs-C"
#define LOG_LEVEL LOG_LEVEL_APP

/* Hardcoded Observed resource path */
static char *obs_res_status_url = "/status";

/* ---------- Network Data Structure ---------- */
static coap_endpoint_t ssd_server_ep;
static coap_observee_t *obs;

int observation_init(const char *server_ep_str){
  if(coap_endpoint_parse(server_ep_str, strlen(server_ep_str), &ssd_server_ep) == 0) {
    LOG_ERR("Failed to parse endpoint: %s\n", server_ep_str);
    return -1;
  }
  return 0;
}

/*
 * Handle the response to the observe request and the following notifications
 */
static void status_notification_callback(coap_observee_t *obs, void *notification,
                      coap_notification_flag_t flag)
{
  int len = 0;
  const uint8_t *payload = NULL;

  LOG_INFO("--> Message from Observed URI: %s\n", obs->url);
  if(notification) {
    len = coap_get_payload(notification, &payload);
  }
  switch(flag) {
  case NOTIFICATION_OK:
    LOG_DBG("Flag: NOTIFICATION_OK\n");
    LOG_DBG("payload: \"%*s\"\n", len, (char *)payload);
    break;
  case OBSERVE_OK: /* server accepeted observation request */
    LOG_DBG("Flag: OBSERVE_OK\n");
    LOG_DBG("payload: \"%*s\"\n", len, (char *)payload);
    break;
  case OBSERVE_NOT_SUPPORTED:
    LOG_DBG("Flag: OBSERVE_NOT_SUPPORTED\n");
    LOG_DBG("payload: \"%*s\"\n", len, (char *)payload);
    obs = NULL;
    break;
  case ERROR_RESPONSE_CODE:
    LOG_DBG("Flag: ERROR_RESPONSE_CODE\n");
    LOG_DBG("payload: \"%*s\"\n", len, (char *)payload);
    obs = NULL;
    break;
  case NO_REPLY_FROM_SERVER:
    LOG_DBG("Flag: NO_REPLY_FROM_SERVER: "
           "removing observe registration with token %x%x\n",
           obs->token[0], obs->token[1]);
    obs = NULL;
    break;
  }
  LOG_DBG("--> Processing new status information\n");
  if((flag == NOTIFICATION_OK || flag == OBSERVE_OK) && payload != NULL) {
    
    unsigned int status_value = 0;
    // Simple JSON parsing (assumes {"status": %u})
    const char *status_str = strstr((const char *)payload, "\"status\":"); //strstr find first occurrency of parameter
    if(status_str != NULL) {
      sscanf(status_str, "\"status\": %u", &status_value);
      set_vent_by_status(status_value);
    } else { LOG_WARN("status value not properly received\n");}
  }
}

void toggle_observation(void){
  if(obs) {
    LOG_INFO("Stopping observation\n");
    coap_obs_remove_observee(obs);
    obs = NULL;
    // set normal status to shutdown all vents
    set_vent_by_status((int) NORMAL_STATUS);
    // switch off alarm system
    leds_off(LEDS_ALL);
  } else {
    LOG_INFO("Starting observation of resource: %s\n", obs_res_status_url);
    obs = coap_obs_request_registration(&ssd_server_ep, obs_res_status_url, status_notification_callback, NULL);
    if(!obs) {
      printf("Observation request failed\n");
    } else {
#ifdef COOJA
    leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
#else
    leds_on(LEDS_BLUE);
#endif    
    }
  }
}

int is_observing(void){
  return obs != NULL;
}


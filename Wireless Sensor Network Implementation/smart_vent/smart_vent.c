#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "lib/network_config.h"
#include "lib/ventilation_control.h"
#include "lib/status_observation_control.h"
#include "sys/etimer.h"
#include "dev/button-hal.h"

/* Log configuration */
#include "sys/log.h"

#define LOG_MODULE "SV"
#define LOG_LEVEL LOG_LEVEL_APP

/* ---------- Exposed CoAP Resources ---------- */
extern coap_resource_t  res_vent;
extern coap_resource_t  res_obs_status;


PROCESS(smart_vent_process, "Smart Vent");
AUTOSTART_PROCESSES(&smart_vent_process);

PROCESS_THREAD(smart_vent_process, ev, data) {
    
  PROCESS_BEGIN();
  
  
  /* --------- Activating CoAP Resource ---------- */
  LOG_INFO("Starting Smart Vent Server\n");
  coap_activate_resource(&res_vent,  "vent");
  coap_activate_resource(&res_obs_status,  "obs_status");
  
  if(observation_init(SSD_SERVER_EP) != 0) {
    PROCESS_EXIT();
  }
  

  while(1) {
    PROCESS_WAIT_EVENT();
    
    /* ============ Button Release Event: Ventilation Shutdown/Restart Simulator ============ */
    if(ev == button_hal_press_event) {
      toggle_observation();
    }
  }
  PROCESS_END();
}


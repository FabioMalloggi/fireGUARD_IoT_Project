#include "contiki.h"
// Log: library
#include "sys/log.h"

/* Log configuration */
#define LOG_MODULE "BR"
#define LOG_LEVEL LOG_LEVEL_INFO

/*---------------------------------*/

PROCESS(br_rplroot_process, "Border Router");
AUTOSTART_PROCESSES(&br_rplroot_process);

/*---------------------------------*/

PROCESS_THREAD(br_rplroot_process, ev, data)
{
	PROCESS_BEGIN(); /*------------*/

	#if BORDER_ROUTER_CONF_WEBSERVER
		PROCESS_NAME(webserver_nogui_process);
		process_start(&webserver_nogui_process, NULL);
	#endif /* BORDER_ROUTER_CONF_WEBSERVER */

	LOG_INFO("Border Router started\n");

	PROCESS_END();
}

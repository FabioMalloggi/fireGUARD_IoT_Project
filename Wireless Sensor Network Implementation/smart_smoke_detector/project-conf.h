#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_


//#define COOJA 1 // FOR LEDs CONFIGURATION

#define LOG_LEVEL_APP LOG_LEVEL_DBG

//#if COOJA
//  #define COAP_MAX_OBSERVERS 12  // For COOJA
//#else
#define COAP_MAX_OBSERVERS 4
//#endif

#define REST_MAX_CHUNK_SIZE 256

#endif /* PROJECT_CONF_H_ */

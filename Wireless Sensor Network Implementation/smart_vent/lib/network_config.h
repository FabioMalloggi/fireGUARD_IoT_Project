#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#define LOCAL_HOST "coap://[fd00::203:3:3:3]/"

// Maximum payload length for all CoAP/observable resources
#define MAX_PAYLOAD_LEN 256

// senML json constants
#define BASE_NAME LOCAL_HOST  // fixed shared base prefix for all sensors
#define VERSION 1
#define NAME_MAX_LEN 16 //suffix name max length
#define UNIT_MAX_LEN 8

// Hardcoded server endpoint
//#define SSD_SERVER_EP "coap://[fe80::202:2:2:2]:5683" //SSD = Smart Smoke Detector
#define SSD_SERVER_EP "coap://[fd00::f6ce:36ed:babb:5620]:5683"

#endif // NETWORK_CONFIG_H

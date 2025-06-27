CONTIKI_PROJECT = smart-smoke-detector
all: $(CONTIKI_PROJECT)
CONTIKI = ../..

# Include project specific Resources
MODULES_REL += ./resources
MODULES_REL += ./lib

# Activate observing module and import it
CFLAGS += -DCOAP_OBSERVE_CLIENT=1
PROJECT_SOURCEFILES += coap-observe-client.c

# Include CoAP module
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"


include $(CONTIKI)/Makefile.include

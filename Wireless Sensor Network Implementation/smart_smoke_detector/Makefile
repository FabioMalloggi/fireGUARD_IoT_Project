CONTIKI_PROJECT = smart-smoke-detector
all: $(CONTIKI_PROJECT)
CONTIKI = ../..

# Include project specific Resources
MODULES_REL += ./resources
# Include project specific libraries
MODULES_REL += ./lib

# Include CoAP module
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap


# Include TinyML emlearn modules
MODULES_REL += /home/iot_ubuntu_intel/.local/lib/python3.10/site-packages/emlearn
TARGET_LIBFILES += -lm
INC += /home/iot_ubuntu_intel/.local/lib/python3.10/site-packages/emlearn


CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"


include $(CONTIKI)/Makefile.include

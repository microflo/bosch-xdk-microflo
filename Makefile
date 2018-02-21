# This makefile triggers the targets in the application.mk

# The default value "../../.." assumes that this makefile is placed in the 
# folder xdk110/Apps/<App Folder> where the BCDS_BASE_DIR is the parent of 
# the xdk110 folder.
BCDS_BASE_DIR ?= ../../..

MICROFLO_GRAPHNAME=main
MICROFLO=./node_modules/.bin/microflo

# Macro to define Start-up method. change this macro to "CUSTOM_STARTUP" to have custom start-up.
export BCDS_SYSTEM_STARTUP_METHOD = DEFAULT_STARTUP
export BCDS_APP_NAME = SendDataOverMQTT
export BCDS_APP_DIR = $(CURDIR)
export BCDS_APP_SOURCE_DIR = $(BCDS_APP_DIR)/source

export XDK_FOTA_ENABLED_BOOTLOADER=1

#Please refer BCDS_CFLAGS_COMMON variable in application.mk file
#and if any addition flags required then add that flags only in the below macro 
export BCDS_CFLAGS_COMMON = -fno-rtti -fno-exceptions

export BCDS_TARGET_BOARD = BSP_XDK110
#List all the application header file under variable BCDS_XDK_INCLUDES
export BCDS_XDK_INCLUDES = \
	-I$(BCDS_APP_SOURCE_DIR)/../node_modules/microflo/microflo
	
#List all the application source file under variable BCDS_XDK_APP_SOURCE_FILES in a similar pattern as below
export BCDS_XDK_APP_SOURCE_FILES = \
	$(BCDS_APP_SOURCE_DIR)/Main.cpp \
	
.PHONY: clean debug release flash_debug_bin flash_release_bin

clean: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk clean

debug: 
	touch source/Main.cpp
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk debug
	
release: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk release
	
clean_Libraries:
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk clean_libraries
	
flash_debug_bin: 
	java -jar ./tools/create_fota_container.jar -i debug/${BCDS_APP_NAME}.bin -fv 00000000 -hv 0100 -pc 0100 -pv 0000
	./tools/xdk-flash /dev/ttyACM0 debug/${BCDS_APP_NAME}.bin
	
flash_release_bin: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk flash_release_bin

cleanlint: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk cleanlint
	
lint: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk lint
	
cdt:
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk cdt

microflo_generate:
	$(MICROFLO) generate --target=xdk --mainfile source/Main.cpp --components ./node_modules/microflo-core/components graphs/${MICROFLO_GRAPHNAME}.fbp build/main.cpp

microflo_runtime:
	$(MICROFLO) runtime --wait-connect 1 --graph graphs/${MICROFLO_GRAPHNAME}.fbp --componentmap build/${MICROFLO_GRAPHNAME}.component.map.json


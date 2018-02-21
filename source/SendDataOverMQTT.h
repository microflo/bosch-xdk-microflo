/*
* Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for Licensee’s application development. 
* Fitness and suitability of the example code for any use within Licensee’s applications need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
* Licensee shall be responsible for conducting the development of its applications as well as integration of parts of the example code into such applications, taking into account the state of the art of technology and any statutory regulations and provisions applicable for such applications. Compliance with the functional system requirements and testing there of (including validation of information/data security aspects and functional safety) and release shall be solely incumbent upon Licensee. 
* For the avoidance of doubt, Licensee shall be responsible and fully liable for the applications and any distribution of such applications into the market.
* 
* 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are 
* met:
* 
*     (1) Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer. 
* 
*     (2) Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.  
*     
*     (3)The name of the author may not be used to
*     endorse or promote products derived from this software without
*     specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR 
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
*  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
*  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
*  POSSIBILITY OF SUCH DAMAGE.
*/
/*----------------------------------------------------------------------------*/

/* header definition ******************************************************** */
#ifndef SENDDATAOVERMQTT_H_
#define SENDDATAOVERMQTT_H_

#include "BCDS_Retcode.h"

/* local interface declaration ********************************************** */
 /* Priorities */

/* local type and macro definitions */

#warning Please enter WLAN configurations with valid SSID & WPA key in below Macros and remove this line to avoid warnings.
/**
 * WIFI_SSID is the WIFI network name where user wants connect the XDK device.
 * Make sure to update the WIFI_SSID constant according to your required WIFI network.
 */
#define WIFI_SSID 			"yourSSID"
/**
 * WIFI_PW is the WIFI router WPA/WPA2 password used at the Wifi network connection.
 * Make sure to update the WIFI_PW constant according to your router password.
 */
#define WIFI_PW				"yourPW"

/**
 * Use an Broker Host address or an IP here
 */
#define MQTT_BROKER_HOST	"yourBrokerHostIP" // use an address or an IP here

/**
 * MQTT_BROKER_PORT is the MQTT server port number.
 */
#define MQTT_BROKER_PORT	UINT16_C(0)

/**
 * DEVICE_NAME is the device name
 */
#define DEVICE_NAME			"XDK_MQTT_EXAMPLE"	//

/**
 * USE_PUBLISH_TIMER is true then application publishes one message per second
 */
#define USE_PUBLISH_TIMER 	true

/**
 * PUBLISH_BUFFER_SIZE is buffer size of publish message
 */
#define PUBLISH_BUFFER_SIZE 256
#define COMMON_BUFFER_SIZE 	PUBLISH_BUFFER_SIZE
#define TOPIC				"BCDS/XDK110/example/out"

/**
 * PUBLISHTIMER_PERIOD_IN_MS is time for MQTT to publish the sensor data
 */
#define PUBLISHTIMER_PERIOD_IN_MS 1000

/**
 * @brief Enumeration to represent the return codes of application.
 */
enum App_Retcode_E
{
    RETCODE_MQTT_PUBLISH_FAIL = RETCODE_FIRST_CUSTOM_CODE,
    RETCODE_TIMER_START_FAIL,
    RETCODE_MQTT_SUBSCRIBE_FAIL,
    RETCODE_MQTT_CONNECT_FAIL,
    RETCODE_MQTT_DISCONNECT_FAIL,
    RETCODE_MQTT_IPCONIG_FAIL,
};

/* local function prototype declarations */

/* local module global variable declarations */

/* local inline function definitions */
/**
 * @brief This is a template function where the user can write his custom application.
 *
 * @param[in] CmdProcessorHandle Handle of the main commandprocessor
 *
 * @param[in] param2  Currently not used will be used in future
 */

extern "C" {

void AppInitSystem(void * CmdProcessorHandle, uint32_t param2);

}

#endif /* SENDDATAOVERMQTT_H_ */

/** ************************************************************************* */

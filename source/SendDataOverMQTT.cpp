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

/* module includes ********************************************************** */

#include <microflo.h>
#include <microflo.cpp>
#include <io.hpp> // TEMP: using NullIO

extern "C" {

#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID  XDK_APP_MODULE_ID_SENDDATAOVERMQTT

/* system header files */
#include "SendDataOverMQTT.h"

#include <stdio.h>
/* additional interface header files */
#include "BCDS_Basics.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"

#include "BCDS_NetworkConfig.h"
#include "BCDS_WlanConnect.h"
#include "BCDS_ServalPal.h"
#include "BCDS_ServalPalWiFi.h"

/* own header files */
#include "BCDS_CmdProcessor.h"
#include "BCDS_Assert.h"
#include "Serval_Mqtt.h"

#include "XdkSensorHandle.h"

}

// XXX: these are defined by xdk110/Libraries/WiFi/3rd-party/TI/simplelink/include/socket.h
#undef connect
#undef send

static void microfloSendToHost(const uint8_t *buf, size_t length);

class MqttHostTransport : public HostTransport {
public:
    MqttHostTransport() {
        
    }

    // implements HostTransport
    virtual void setup(IO *i, HostCommunication *c) {
        controller = c;
    }
    virtual void runTick() {
        // no-op, everything happens event-oriented
        // FIXME: implement receiving
    }
    virtual void sendCommand(const uint8_t *buf, uint8_t len) {
        // Pad to a whole command
        uint8_t cmd[MICROFLO_CMD_SIZE];
        for (uint8_t i=0; i<MICROFLO_CMD_SIZE; i++) {
            cmd[i] = (i < len) ?  buf[i] : 0x00;
        }

        microfloSendToHost(cmd, MICROFLO_CMD_SIZE);
    }

private:
    HostCommunication *controller;
};


/* constant definitions ***************************************************** */

/* local variables ********************************************************** */

NullIO io;
MqttHostTransport transport;
FixedMessageQueue queue;
Network network(&io, &queue);
HostCommunication controller;

static CmdProcessor_T *AppCmdProcessor;
static CmdProcessor_T CmdProcessorHandleServalPAL;
static MqttSession_T Session;
static MqttSession_T *SessionPtr = 0;

static char PublishBuffer[PUBLISH_BUFFER_SIZE];
static uint8_t PublishInProgress = 0;

static TimerHandle_t PublishTimerHandle;
static TimerHandle_t MicrofloTimerHandle;

static const char *PublishTopic = TOPIC_PREFIX"/out";
static const char *SubscribeTopic = TOPIC_PREFIX"/in";
static const char *microfloSendTopic = TOPIC_PREFIX"/microflo/send";
static const char *microfloReceiveTopic = TOPIC_PREFIX"/microflo/receive";

static StringDescr_T PublishTopicDescription;
static StringDescr_T SubscribeTopicDescription;
static StringDescr_T microfloSendTopicDescription;
static StringDescr_T microfloReceiveTopicDescription;

const int N_SUBSCRIBE_TOPICS = 2;
static StringDescr_T SubscribeTopics[N_SUBSCRIBE_TOPICS];
static Mqtt_qos_t SubscribeQos[N_SUBSCRIBE_TOPICS] = {
    MQTT_QOS_AT_MOST_ONE,
    MQTT_QOS_AT_MOST_ONE,
};

static char MqttBroker[50];
static const char MqttBrokerAddressFormat[50] = "mqtt://%s:%d";
static const char *DeviceName = DEVICE_NAME;


/* global variables ********************************************************* */

/* inline functions ********************************************************* */

/* local functions ********************************************************** */

/**
 * @brief This will initialize the ServalPAL module and its necessary resources.
 *
 * @return Based on the status of #ServalPal_Initialize, #ServalPalWiFi_Init and #ServalPalWiFi_NotifyWiFiEvent
 */

static Retcode_T ServalPalSetup(void)
{
    Retcode_T returnValue = RETCODE_OK;
    returnValue = CmdProcessor_Initialize(&CmdProcessorHandleServalPAL, (char *)"Serval PAL", TASK_PRIORITY_SERVALPAL_CMD_PROC, TASK_STACK_SIZE_SERVALPAL_CMD_PROC, TASK_QUEUE_LEN_SERVALPAL_CMD_PROC);
    /* serval pal common init */
    if (RETCODE_OK == returnValue)
    {
        returnValue = ServalPal_Initialize(&CmdProcessorHandleServalPAL);
    }
    if (RETCODE_OK == returnValue)
    {
        returnValue = ServalPalWiFi_Init();
    }
    if (RETCODE_OK == returnValue)
    {
        ServalPalWiFi_StateChangeInfo_T stateChangeInfo = { SERVALPALWIFI_OPEN, 0 };
        returnValue = ServalPalWiFi_NotifyWiFiEvent(SERVALPALWIFI_STATE_CHANGE, &stateChangeInfo);
    }
    return returnValue;
}

/**
 * @brief To initialize the network setup
 *
 * @retval #RETCODE_OK Network initializes successfully
 * @return In case any other error refer #WlanConnect_Init, #NetworkConfig_SetIpDhcp, #WlanConnect_WPA and #ServalPalSetup error codes.
 */
static Retcode_T NetworkSetup(void)
{
    Retcode_T retcode = RETCODE_OK;
    WlanConnect_SSID_T connectSSID = (WlanConnect_SSID_T) WIFI_SSID;
    WlanConnect_PassPhrase_T connectPassPhrase =
            (WlanConnect_PassPhrase_T) WIFI_PW;
    retcode = WlanConnect_Init();
    if (retcode == RETCODE_OK)
    {
        retcode = NetworkConfig_SetIpDhcp(0);

    }
    if (retcode == RETCODE_OK)
    {
        retcode = WlanConnect_WPA(connectSSID, connectPassPhrase, NULL);
    }
    if (retcode == RETCODE_OK)
    {
        retcode = ServalPalSetup();
    }

    return retcode;
}

/**
 * @brief To read environmental sensor data
 *
 * @return Environmental sensor data in standard units
 */
static Environmental_Data_T ReadEnvironmentSensor(void)
{

    Environmental_Data_T bme280 = { 0, 0, 0 };
    Retcode_T retcode = Environmental_readData(xdkEnvironmental_BME280_Handle, &bme280);
    if (RETCODE_OK != retcode)
    {
        bme280.humidity = 0;
        bme280.pressure = 0;
        bme280.temperature = 0;
    }
    return bme280;
}

/**
 * @brief To initialize environmental BME280 sensor driver
 *
 */
static void InitEnvironmentalSensor(void)
{
    Retcode_T retcode = Environmental_init(xdkEnvironmental_BME280_Handle);
    if (RETCODE_OK != retcode)
    {
        printf("Environmental_init is failed \n\r");
        Retcode_RaiseError(retcode);
    }
}






/**
 * @brief MQTT subscribe to own public topic
 *
 * @return Based on status of #Mqtt_subscribe
 */
static retcode_t SubscribeToTopic(void)
{
    printf("Subscribing to topics: '%s', '%s'\n\r",
        SubscribeTopic,
        microfloReceiveTopic
    );
    retcode_t one = Mqtt_subscribe(SessionPtr, 1, SubscribeTopics, SubscribeQos);
    if (one != RC_OK) {
        return one;
    } 
    retcode_t two = Mqtt_subscribe(SessionPtr, 1, &SubscribeTopics[1], &SubscribeQos[1]);
    return two;
}


//static void MicrofloSendToHost(void *param1, uint32_t param2) {
//}

static void microfloSendToHost(const uint8_t *buf, size_t length) {
    memcpy(PublishBuffer, buf, length);

    if (!SessionPtr) {
        printf("cannot send, MQTT session not setup yet\n\r");
        return;
    }

    const retcode_t rc_publish = Mqtt_publish(SessionPtr, microfloSendTopicDescription,
            PublishBuffer, length, (uint8_t) MQTT_QOS_AT_MOST_ONE, false);
    if (rc_publish == RC_OK)
    {
        PublishInProgress = 1;
    }
    else
    {
        PublishInProgress = 0;
        printf("Mqtt_publish is failed:Stack erro code :%u \n\r",(unsigned int)rc_publish);
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_PUBLISH_FAIL));
    }
}


static void RunMicroflo(TimerHandle_t pxTimer)
{
    BCDS_UNUSED(pxTimer);

    printf("%s", ".");

    transport.runTick();
    network.runTick();
}

static void CreateMicrofloTimer(void)
{
    MicrofloTimerHandle = xTimerCreate(
            (const char * const ) "MicroFlo Timer",
            (100/portTICK_RATE_MS),
            pdTRUE,
            NULL,
            RunMicroflo);

    if(NULL == MicrofloTimerHandle)
    {
        printf("microflo xTimerCreate is failed \n\r");
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_OUT_OF_RESOURCES));
    }
    else if ( pdFAIL == xTimerStart(MicrofloTimerHandle, 10000))
    {
        printf("microflo xTimerStart is failed \n\r");
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_TIMER_START_FAIL));
    }
}

static void HandleEventConnection(MqttConnectionEstablishedEvent_T connectionData)
{
    printf("connected %d, %d\n\r",
            (int) connectionData.connectReturnCode,
            (int) connectionData.sessionPresentFlag);
    if (connectionData.connectReturnCode == 0)
    {
        retcode_t rc = SubscribeToTopic();
        if(RC_OK != rc)
        {
            printf("SubscribeToOwnPublishTopic is failed\n\r");
            printf("Stack error code :%u \n\r",(unsigned int)rc);
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_SUBSCRIBE_FAIL));
        }
    }

}


static void HandleEventIncomingData(MqttPublishData_T publishData)
{
    char published_topic_buffer[COMMON_BUFFER_SIZE] = {0};
    char published_data_buffer[COMMON_BUFFER_SIZE] = {0};
    static int incoming_message_count = 0;

    strncpy(published_data_buffer, (const char *)publishData.payload ,sizeof(published_data_buffer));
    strncpy(published_topic_buffer, publishData.topic.start, sizeof(published_topic_buffer));

    printf("received #%d : %s : %d %s \n\r",
            incoming_message_count, published_topic_buffer,
            publishData.length, published_data_buffer
    );
    incoming_message_count++;

    // FIXME: don't assume its on the microflo topic
    for (unsigned int i=0; i<publishData.length; i++) {
        unsigned char c = publishData.payload[i];
        controller.parseByte(c);
    }
}


static void HandleEventSuccessfulPublish(void *param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);
    PublishInProgress = 0;
}

/* global functions ********************************************************* */

/**
 * @brief MQTT even handler to process various MQTT events.
 *
 * @param[in] session
 * Unused
 *
 * @param[in] event
 * MQTT event
 *
 * @param[in] eventData
 * Event data type of MQTT
 *
 * @retval #RC_OK
 */

static retcode_t EventHandler(MqttSession_T* session, MqttEvent_t event,
        const MqttEventData_t* eventData)
{
    BCDS_UNUSED(session);
    Retcode_T retcode = RETCODE_OK;

    switch (event)
    {
    case MQTT_CONNECTION_ESTABLISHED:
        HandleEventConnection(eventData->sl_Connect);
        break;
    case MQTT_CONNECTION_ERROR:
        HandleEventConnection(eventData->sl_Connect);
        break;
    case MQTT_INCOMING_PUBLISH:
        HandleEventIncomingData(eventData->publish);
        break;
    case MQTT_PUBLISHED_DATA:

        retcode = CmdProcessor_Enqueue(AppCmdProcessor, HandleEventSuccessfulPublish, NULL, 0);
        if (RETCODE_OK != retcode)
        {
            printf("CmdProcessor_Enqueue is failed \n\r");
            Retcode_RaiseError(retcode);
        }
        break;
    case MQTT_SUBSCRIBE_SEND_FAILED:
        printf("MQTT_SUBSCRIBE_SEND_FAILED\n\r");
        break;
    case MQTT_SUBSCRIPTION_ACKNOWLEDGED:
        printf("MQTT_SUBSCRIPTION_ACKNOWLEDGED\n\r");
        break;
    case MQTT_CONNECT_TIMEOUT:
        printf("MQTT_CONNECT_TIMEOUT\n\r");
        break;
    default:
        printf("Unhandled MQTT Event Number: %d\n\r", event);
        break;
    }
    return RC_OK;
}

/**
 * @brief To disconnect the MQTT session
 *
 * @return Based on status of #Mqtt_disconnect
 */
static retcode_t Disconnect(void)
{
    retcode_t rc = RC_INVALID_STATUS;
    rc = Mqtt_disconnect(SessionPtr);
    if (rc != RC_OK)
    {
        printf("Could not Disconnect, error 0x%04x\n", rc);
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_DISCONNECT_FAIL));
    }
    return rc;
}

/**
 * @brief To start the MQTT application
 *
 */
static void StartExample(void)
{
    retcode_t rc = RC_INVALID_STATUS;
    rc = Mqtt_connect(SessionPtr);
    if (rc != RC_OK)
    {
        printf("Could not Connect, error 0x%04x\n", rc);
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_CONNECT_FAIL));
    }
}

/**
 * @brief To configure the MQTT session
 *
 */
static void ConfigureSession(void)
{
    // set target
    int8_t server_ip_buffer[13];
    Ip_Address_T ip;

    Retcode_T rc;
    rc = NetworkConfig_GetIpAddress((uint8_t *) MQTT_BROKER_HOST, &ip);
    if(RETCODE_OK != rc)
    {
        printf("Getting Broker address failure\n\r");
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_IPCONIG_FAIL));
    }
    else
    {
        int32_t retval = Ip_convertAddrToString(&ip, (char *)server_ip_buffer);
        if(0 == retval)
        {
            printf("Ip_convertAddrToString return value is zero\n\r");
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_IPCONIG_FAIL));
        }
        else
        {
            sprintf(MqttBroker, MqttBrokerAddressFormat, server_ip_buffer,
            MQTT_BROKER_PORT);

            rc = SupportedUrl_fromString((const char *)MqttBroker, (uint16_t) strlen((const char *)MqttBroker),
                    &SessionPtr->target);
            if(RC_OK != rc)
            {
                printf("SupportedUrl_fromString failure\n\r");
                Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_IPCONIG_FAIL));
            }
            else
            {
                SessionPtr->target.scheme = SERVAL_SCHEME_MQTT;

                printf("Broker address: %s\n\r", MqttBroker);

                // set event handler
                SessionPtr->onMqttEvent = EventHandler;

                // set Connect information
                SessionPtr->MQTTVersion = 3;
                SessionPtr->keepAliveInterval = 100;
                SessionPtr->cleanSession = true;
                SessionPtr->will.haveWill = false;

                StringDescr_T device_name_descr;
                StringDescr_wrap(&device_name_descr, DeviceName);
                SessionPtr->clientID = device_name_descr;

                // set publish and subscribe Topics as StringDescr
                StringDescr_wrap(&PublishTopicDescription, (const char *)PublishTopic);
                StringDescr_wrap(&SubscribeTopicDescription, (const char *)SubscribeTopic);
                StringDescr_wrap(&microfloSendTopicDescription, (const char *)microfloSendTopic);
                StringDescr_wrap(&microfloReceiveTopicDescription, (const char *)microfloReceiveTopic);

                StringDescr_wrap(&(SubscribeTopics[0]), SubscribeTopic);
                StringDescr_wrap(&(SubscribeTopics[1]), microfloReceiveTopic);
            }
        }
    }
}

/**
 * @brief To initialize the MQTT application
 *
 */
static void InitializeExample(void)
{
    retcode_t rc_initialize = Mqtt_initialize();
    if (rc_initialize != RC_OK)
    {
        printf("Could not initialize mqtt, error 0x%04x\n", rc_initialize);
    }
    else
    {
        SessionPtr = &Session;
        memset(SessionPtr, 0, sizeof(*SessionPtr));
        rc_initialize = Mqtt_initializeInternalSession(SessionPtr);
        if (rc_initialize != RC_OK)
        {
            printf("Mqtt_initializeInternalSession is failed\n\r");
        }
        else
        {
            InitEnvironmentalSensor();
            ConfigureSession();
            StartExample();
        }
    }
}

extern "C" {

#ifndef MICROFLO_EMBED_GAPH
//#error "no graph to load"
// FIXME: should not be needed
#define MICROFLO_EMBED_GAPH
#endif

void AppInitSystem(void * cmdProcessorHandle, uint32_t param2)
{

    transport.setup(&io, &controller);
    controller.setup(&network, &transport);

    MICROFLO_LOAD_STATIC_GRAPH((&controller), graph);

    network.subscribeToPort(2, 0, true); // TEMP: to get some data produced when running

    if (cmdProcessorHandle == NULL)
    {
        printf("Command processor handle is null \n\r");
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_FATAL,RETCODE_NULL_POINTER));
    }
    BCDS_UNUSED(param2);
    AppCmdProcessor = (CmdProcessor_T *) cmdProcessorHandle;

    Retcode_T connect_rc = NetworkSetup();
    if (connect_rc == RETCODE_OK)
    {
        InitializeExample();
    }
    else
    {
        printf("Connect Failed, so example is not started\n\r");
        Retcode_RaiseError(connect_rc);
    }


    CreateMicrofloTimer();
}

} // end extern "C"

/** ************************************************************************* */

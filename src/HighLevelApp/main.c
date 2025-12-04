#include <applibs/log.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include "applibs_versions.h"
#include <applibs/eventloop.h>
#include <applibs/networking.h>
#include <applibs/log.h>
#include "common/cloud.h"
#include "common/eventloop_timer_utilities.h"
#include "common/azure_iot.h"
#include "common/connection.h"
#include "common/connection_iot_hub.h"
#include "common/parson.h"
#include <applibs/application.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iothub_device_client_ll.h>
#include <iothubtransportmqtt.h>
#include <iothub_client_options.h>
#include <iothub.h>

#include <inttypes.h>

#include "azure_macro_utils/macro_utils.h"
#include "iothub_client_options.h"
#include "iothub.h"
#include "iothub_message.h"


static const struct timespec interval = {
    .tv_sec = 0,
    .tv_nsec = 100 * 1000 * 1000
};
static IOTHUB_DEVICE_CLIENT_LL_HANDLE client;
AzureIoT_Callbacks call;

static const char* connectionString = "HostName=PIIOTFree.azure-devices.net;DeviceId=830cdd8aa23ff106502f452e27b7f0992d2fb0d1152f473e90277c8115a2407530940b794b05ab06713ae900a31abaa1ae453d4cea113c5e9d241bd16e3d76aa;x509=true";
static int deviceMethodCallback(const char *methodName,
                                const unsigned char *payload,
                                size_t payloadSize,
                                unsigned char **response,
                                size_t *responseSize) {
    const char resp[] = "{ \"result\": \"ok\" }";
    *responseSize = sizeof(resp) - 1;
    *response = malloc(*responseSize);
    memcpy(*response, resp, *responseSize);
    return 200;
}



static void twin_method(void){

    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocoll;


    protocoll = MQTT_Protocol;

    if(IoTHub_Init() != 0){
        Log_Debug("Failed to init");
    }

    else
    {
        if((client = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString,protocoll)) == NULL){
            Log_Debug("ERROR: Client Handle  is NULL\r\n");
        }
        else
        {
            bool urlEnocdeOn = true;
            Log_Debug("Created client=%p\n", client);

            IOTHUB_CLIENT_RESULT res_1 = IoTHubDeviceClient_LL_SetOption(client, OPTION_AUTO_URL_ENCODE_DECODE, &urlEnocdeOn);
            
            IOTHUB_CLIENT_RESULT res =  IoTHubDeviceClient_LL_SetDeviceMethodCallback(client,deviceMethodCallback,NULL);

            if(res_1 != IOTHUB_CLIENT_OK || res != IOTHUB_CLIENT_OK){
                Log_Debug("Client init failed\n");
            }else{
                Log_Debug("Initialisierung direct method war erfolgreich\n");

            }
            
        }
        
    }



}



typedef struct{
    int distance;
}Message;



static int sockFd = -1;
static EventRegistration *socketEventReg = NULL;
static EventLoop *eventLoop = NULL;
static volatile sig_atomic_t exitCode = ExitCode_Success;

static Connection_IotHub_Config config = {.hubHostname = "PIIOTFree.azure-devices.net"};
static const char rtAppComponentId[] = "6b2de05f-eb0f-4433-90e5-74eb950f35c4";




AzureIoT_Result Send_Message (Message message){
    Log_Debug("Versende Nachricht");
    
    JSON_Value *value = json_value_init_object();
    JSON_Object *root = json_value_get_object(value);
    json_object_dotset_number(root,"distance",message.distance);
    char* serializedMessage = json_serialize_to_string(value);
    AzureIoT_Result result  = AzureIoT_SendTelemetry(serializedMessage,NULL,NULL);
    

    json_free_serialized_string(serializedMessage);
    json_value_free(value);
    
    return result;

}



static void SocketEventHandler(EventLoop *el, int fd, EventLoop_IoEvents events, void *context) {
    char rxBuf[32];
    int bytesReceived = recv(fd, rxBuf, sizeof(rxBuf), 0);
    Message mes;

    if (bytesReceived == -1) {
        Log_Debug("ERROR: Unable to receive message: %d (%s)\n", errno, strerror(errno));
        exitCode = -1;
        return;
    }

    if (bytesReceived == 2) {
    uint16_t value =
    ((uint16_t)(uint8_t)rxBuf[1] << 8) |
    (uint16_t)(uint8_t)rxBuf[0];


    Log_Debug("Wert: %u\n", value);
    mes.distance = value;
    //Send_Message(mes);
    }
    Log_Debug("\n");

}

static ExitCode SetupEventLoop(void)
{
    Log_Debug("Starting Event Loop Setup\n");

    if (eventLoop == NULL)
    {
        Log_Debug("EventLoop konnte nicht erstellt werden\n");
        return ExitCode_Init_EventLoop;
    }

    sockFd = Application_Connect(rtAppComponentId);
    if (sockFd == -1) return -1;

    static const struct timeval recvTimeout = {.tv_sec = 5, .tv_usec = 0};
    if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(recvTimeout)) == -1)
        return -2;

    socketEventReg = EventLoop_RegisterIo(eventLoop, sockFd, EventLoop_Input, SocketEventHandler, NULL);
    if (!socketEventReg) return -3;
    if (sockFd == -1) return -4;

    
    return ExitCode_Success;
}

static void ExitCodeCallbackHandler(ExitCode ec)
{
    exitCode = ec;
}

static void sende_daten(bool success, void *context)
{
    Log_Debug("Gesendet");
};

AzureIoT_Callbacks callbacks = {
    .sendTelemetryCallbackFunction = sende_daten,
    .deviceMethodCallbackFunction = deviceMethodCallback,
};


int main(void)
{
    Log_Debug("Starting IoT Hub Connection\n");
    void *connectionContext = (void *)&config;
    bool isNetworkingReady = false;
    if ((Networking_IsNetworkingReady(&isNetworkingReady) == -1) || !isNetworkingReady) {
        Log_Debug("WARNING: Network is not ready. Device cannot connect until network is ready.\n");
    }

    eventLoop = EventLoop_Create();
    if (!eventLoop) {
        Log_Debug("Failed to create EventLoop\n");
        return ExitCode_Init_EventLoop;
    }

    ExitCode code = AzureIoT_Initialize(eventLoop, ExitCodeCallbackHandler, NULL, connectionContext, callbacks);
    if (code != ExitCode_Success) {
        Log_Debug("AzureIoT_Initialize failed\n");
        return code;
    }

    Log_Debug("AzureIoT Init erfolgreich\n");
    //twin_method(); 

    ExitCode timerCode = SetupEventLoop();
    if (timerCode != ExitCode_Success) {
        exitCode = timerCode;
    }

    while (exitCode == ExitCode_Success) {
        EventLoop_Run_Result result = EventLoop_Run(eventLoop, -1, true);
        if (result == EventLoop_Run_Failed) {
            exitCode = ExitCode_Main_EventLoopFail;
        }
    }

    AzureIoT_Cleanup();
    IoTHubDeviceClient_LL_Destroy(client);
    IoTHub_Deinit();
    EventLoop_Close(eventLoop);
    Log_Debug("Exiting application with code %d\n", exitCode);
    return exitCode;
}

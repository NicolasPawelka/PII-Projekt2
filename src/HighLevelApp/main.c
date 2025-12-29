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
#include<stdio.h>

#include "azure_macro_utils/macro_utils.h"
#include "iothub_client_options.h"
#include "iothub.h"
#include "iothub_message.h"

AzureIoT_Callbacks call;
typedef struct{
    int distance;
    int acceleration;
}Message;



static int sockFd = -1;
static EventRegistration *socketEventReg = NULL;
static EventLoop *eventLoop = NULL;
static volatile sig_atomic_t exitCode = ExitCode_Success;
bool send_Flag = false;

static Connection_IotHub_Config config = {.hubHostname = "PIIOTFree.azure-devices.net"};
static const char rtAppComponentId[] = "6b2de05f-eb0f-4433-90e5-74eb950f35c4";

static void SendMessageToRTApp(void) {
    static int iter = 0;
    static char txMessage[32];
    snprintf(txMessage, sizeof(txMessage), "hl-app-to-rt-app-%02d", iter);
    iter = (iter + 1) % 100;

    Log_Debug("Sending: %s\n", txMessage);

    int bytesSent = send(sockFd, txMessage, strlen(txMessage), 0);
    if (bytesSent == -1) {
        Log_Debug("ERROR: Unable to send message: %d (%s)\n", errno, strerror(errno));
        exitCode = -1;
        return;
    }
    Log_Debug("Gesendet");
}

static int deviceMethodCallback(const char *methodName,
                                const unsigned char *payload,
                                size_t payloadSize,
                                unsigned char **response,
                                size_t *responseSize) {
    send_Flag = true;
    const char resp[] = "{ \"result\": \"AllesRoger\" }";
    *responseSize = sizeof(resp) - 1;
    *response = malloc(*responseSize);
    memcpy(*response, resp, *responseSize);
    return 200;
}





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
    Send_Message(mes);
    }
    //Log_Debug("\n");

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
    IoTHub_Deinit();
    EventLoop_Close(eventLoop);
    Log_Debug("Exiting application with code %d\n", exitCode);
    return exitCode;
}

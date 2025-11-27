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

typedef struct{
    int distance;
}Message;



static int sockFd = -1;
static EventRegistration *socketEventReg = NULL;
IOTHUB_DEVICE_CLIENT_LL_HANDLE iothubClientHandle;
static EventLoop *eventLoop = NULL;
static EventLoopTimer *timer = NULL;
static volatile sig_atomic_t exitCode = ExitCode_Success;

static Connection_IotHub_Config config = {.hubHostname = "PIIOT-Hub.azure-devices.net"};
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
    Send_Message(mes);
    }
    Log_Debug("\n");

}


// static void TimerCallback(EventLoopTimer *timer)
// {
//      if (ConsumeEventLoopTimerEvent(timer) != 0) {
//         exitCode = ExitCode_TelemetryTimer_Consume;
//         return;
//     }
//     Log_Debug("Versende Nachricht....");
//     AzureIoT_Result result = Send_Message();
    

//      if(result  != AzureIoT_Result_OK){
//         Log_Debug("Versenden der Nachricht fehlgescchlagen!\n");
//     }
// }

static ExitCode SetupEventLoop(void)
{
    Log_Debug("Starting Event Loop Setup\n");
    struct timespec timePeriod = {.tv_sec = 20, .tv_nsec = 0};

    if (eventLoop == NULL)
    {
        Log_Debug("EventLoop konnte nicht erstellt werden\n");
        return ExitCode_Init_EventLoop;
    }

    // Log_Debug("Event Loop wurde erstellt. Erstelle Timer...\n");
    // timer = CreateEventLoopPeriodicTimer(eventLoop, &TimerCallback, &timePeriod);
    sockFd = Application_Connect(rtAppComponentId);
    if (sockFd == -1) return -1;

    static const struct timeval recvTimeout = {.tv_sec = 5, .tv_usec = 0};
    if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(recvTimeout)) == -1)
        return -2;

    socketEventReg = EventLoop_RegisterIo(eventLoop, sockFd, EventLoop_Input, SocketEventHandler, NULL);
    if (!socketEventReg) return -3;
    if (sockFd == -1) return -4;
    // if (timer == NULL)
    // {
    //     return ExitCode_Init_TelemetryTimer;
    // }

    // Log_Debug("Timer wurde gestartet\n");
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
};

int main(void)
{
    Log_Debug("Starting Iot Hub Connection\n");
    void *connectionContext = (void *)&config;
    bool isNetworkingReady = false;
    if ((Networking_IsNetworkingReady(&isNetworkingReady) == -1) || !isNetworkingReady)
    {
        Log_Debug("WARNING: Network is not ready. Device cannot connect until network is ready.\n");
    }

    eventLoop = EventLoop_Create();
    ExitCode code = AzureIoT_Initialize(eventLoop, ExitCodeCallbackHandler, NULL, connectionContext, callbacks);
    if (code == ExitCode_Success)
    {
        Log_Debug("AzureIot Init war erfolgreich\n");

        ExitCode timerCode = SetupEventLoop();
        if (timerCode != ExitCode_Success)
        {
            exitCode = timerCode;
        }

        while (exitCode == ExitCode_Success)
        {
            EventLoop_Run_Result result = EventLoop_Run(eventLoop, -1, true);
            if (result == EventLoop_Run_Failed)
            {
                exitCode = ExitCode_Main_EventLoopFail;
            }
        }

        DisposeEventLoopTimer(timer);
        AzureIoT_Cleanup();
        EventLoop_Close(eventLoop);
        Log_Debug("Exiting application with code %d\n", exitCode);
        return exitCode;
    }
}
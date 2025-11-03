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

IOTHUB_DEVICE_CLIENT_LL_HANDLE iothubClientHandle;
static EventLoop *eventLoop = NULL;
static EventLoopTimer *timer = NULL;
static volatile sig_atomic_t exitCode = ExitCode_Success;

static Connection_IotHub_Config config = {.hubHostname = "PIIOT-Hub.azure-devices.net"};

typedef struct{
    int distance;
}Message;

AzureIoT_Result Send_Message (void){
    int message = rand() % (100 + 1);
    
    JSON_Value *value = json_value_init_object();
    JSON_Object *root = json_value_get_object(value);
    json_object_dotset_number(root,"distance",message);
    char* serializedMessage = json_serialize_to_string(value);
    AzureIoT_Result result  = AzureIoT_SendTelemetry(serializedMessage,NULL,NULL);
    

    json_free_serialized_string(serializedMessage);
    json_value_free(value);
    
    return result;

}


static void TimerCallback(EventLoopTimer *timer)
{
     if (ConsumeEventLoopTimerEvent(timer) != 0) {
        exitCode = ExitCode_TelemetryTimer_Consume;
        return;
    }
    Log_Debug("Versende Nachricht....");
    AzureIoT_Result result = Send_Message();

     if(result  != AzureIoT_Result_OK){
        Log_Debug("Versenden der Nachricht fehlgescchlagen!\n");
    }
}

static ExitCode SetupEventLoop(void)
{
    Log_Debug("Starting Event Loop Setup\n");
    struct timespec timePeriod = {.tv_sec = 20, .tv_nsec = 0};

    if (eventLoop == NULL)
    {
        Log_Debug("EventLoop konnte nicht erstellt werden\n");
        return ExitCode_Init_EventLoop;
    }

    Log_Debug("Event Loop wurde erstellt. Erstelle Timer...\n");
    timer = CreateEventLoopPeriodicTimer(eventLoop, &TimerCallback, &timePeriod);
    if (timer == NULL)
    {
        return ExitCode_Init_TelemetryTimer;
    }

    Log_Debug("Timer wurde gestartet\n");
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
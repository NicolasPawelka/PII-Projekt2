// This minimal Azure Sphere app prints "High Level Application" to the debug
// console and exits with status 0.

#include <applibs/log.h>
#include <applibs/gpio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <applibs/log.h>
#include <applibs/gpio.h>

typedef enum {
    ExitCode_Success = 0,

    ExitCode_Main_Led = 1
} ExitCode;
int main(void)
{
    // Please see the extensible samples at: https://github.com/Azure/azure-sphere-samples
    // for examples of Azure Sphere platform features
    Log_Debug("High Level Application\n");
    GPIO_Value_Type* val;
    int fd = GPIO_OpenAsOutput(8, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (fd < 0) {
        Log_Debug(
            "Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
            strerror(errno), errno);
        return ExitCode_Main_Led;
    }

    const struct timespec sleepTime = {.tv_sec = 1, .tv_nsec = 0};
    while(1){

   

    GPIO_SetValue(fd, GPIO_Value_Low);
    nanosleep(&sleepTime, NULL);
    GPIO_SetValue(fd, GPIO_Value_High);
    nanosleep(&sleepTime, NULL);

    }
    return 0;
}
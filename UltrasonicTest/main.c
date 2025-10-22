#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <applibs/log.h>
#include <applibs/gpio.h>
#include <hw/template_appliance.h>

typedef enum {
    ExitCode_Success = 0,
    ExitCode_Main_Led = 1,
    ExitCode_Main_Sensor= 2
} ExitCode;

//Trigger und Echo auf einem einzigen Pin
#define SIG_PIN  MT3620_RDB_HEADER1_PIN4_GPIO


//die vier Abstands-LEDs definieren
#define RED_LED1 MT3620_RDB_LED1_RED
#define RED_LED2 MT3620_RDB_LED2_RED
#define RED_LED3 MT3620_RDB_LED3_RED
#define RED_LED4 MT3620_RDB_LED4_RED

uint64_t measure_pulse_duration(int fdSig);
void busy_wait_us(uint32_t us);




int main(void) {
    // Rote LEDs öffnen
    int fdRed1  = GPIO_OpenAsOutput(RED_LED1, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    int fdRed2  = GPIO_OpenAsOutput(RED_LED2, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    int fdRed3  = GPIO_OpenAsOutput(RED_LED3, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    int fdRed4  = GPIO_OpenAsOutput(RED_LED4, GPIO_OutputMode_PushPull, GPIO_Value_Low);

    //Signal-Pin als Output
    int fdSig = GPIO_OpenAsOutput(SIG_PIN, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    if (fdSig < 0) {
        Log_Debug("Fehler beim Öffnen des SIG-Pins: %s (%d)\n", strerror(errno), errno);
        return ExitCode_Main_Sensor;
    }

    const struct timespec sleepTime = {.tv_sec = 0, .tv_nsec = 300000000}; // 300 ms Pause
while(true){

    GPIO_SetValue(fdSig, GPIO_Value_Low);
    busy_wait_us(2);
    GPIO_SetValue(fdSig, GPIO_Value_High);
    busy_wait_us(10);
    GPIO_SetValue(fdSig, GPIO_Value_Low);


    close(fdSig);
    fdSig = GPIO_OpenAsInput(SIG_PIN);
    if (fdSig < 0) {
            Log_Debug("Fehler beim Umschalten auf Input: %s (%d)\n", strerror(errno), errno);
            return ExitCode_Main_Sensor;
    }

    
    uint64_t pulse_us = measure_pulse_duration(fdSig);

    close(fdSig);
    fdSig = GPIO_OpenAsOutput(SIG_PIN, GPIO_OutputMode_PushPull, GPIO_Value_Low);

    float distance_cm = (pulse_us > 0) ? (pulse_us / 58.0f) : -1.0f; //müsste man noch kalibrieren
    Log_Debug("Entfernung: %.1f cm\n", distance_cm);


    // LEDs steuern, eine nach der anderen soll angehen, je näher man kommt. 
        if (distance_cm > 40.0f ) {
            GPIO_SetValue(fdRed1, GPIO_Value_High); // High= LED aus; Low= LED an!
            GPIO_SetValue(fdRed2,  GPIO_Value_High);
            GPIO_SetValue(fdRed3, GPIO_Value_High);
            GPIO_SetValue(fdRed4,  GPIO_Value_High);
        }
        else if (distance_cm < 40.0f && distance_cm >= 30.0f){
            GPIO_SetValue(fdRed1, GPIO_Value_Low); 
            GPIO_SetValue(fdRed2,  GPIO_Value_High);
            GPIO_SetValue(fdRed3, GPIO_Value_High);
            GPIO_SetValue(fdRed4,  GPIO_Value_High);
        
        } 
        else if (distance_cm < 30.0f && distance_cm >=20.0f){
            GPIO_SetValue(fdRed1, GPIO_Value_Low);
            GPIO_SetValue(fdRed2,  GPIO_Value_Low);
            GPIO_SetValue(fdRed3, GPIO_Value_High);
            GPIO_SetValue(fdRed4,  GPIO_Value_High);
            
        }
        else if (distance_cm < 20.0f &&  distance_cm>=10.0f){
            GPIO_SetValue(fdRed1, GPIO_Value_Low);
            GPIO_SetValue(fdRed2,  GPIO_Value_Low);
            GPIO_SetValue(fdRed3, GPIO_Value_Low);
            GPIO_SetValue(fdRed4,  GPIO_Value_High);    

        }
        else if (distance_cm < 10.0f){
            GPIO_SetValue(fdRed1, GPIO_Value_Low);
            GPIO_SetValue(fdRed2,  GPIO_Value_Low);
            GPIO_SetValue(fdRed3, GPIO_Value_Low);
            GPIO_SetValue(fdRed4,  GPIO_Value_Low);
         }

         nanosleep(&sleepTime, NULL);

}  
    return ExitCode_Success;
}



// Funktion für die Zeitmessung des Pulses
uint64_t measure_pulse_duration(int fdSig) {
    struct timespec ts;
    uint64_t start_time = 0, end_time = 0;
    GPIO_Value_Type val;

    const uint64_t timeout_us = 30000; // Timeout in µs

    // Warte auf HIGH
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t start_wait = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;

    while (true) {
        GPIO_GetValue(fdSig, &val);
        if (val == GPIO_Value_High) {
            clock_gettime(CLOCK_MONOTONIC, &ts);
            start_time = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
            break;
        }
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint64_t now_us = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
        if (now_us - start_wait > timeout_us) return 0; // Timeout
    }

    // Warte auf LOW
    while (true) {
        GPIO_GetValue(fdSig, &val);
        if (val == GPIO_Value_Low) {
            clock_gettime(CLOCK_MONOTONIC, &ts);
            end_time = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
            break;
        }
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint64_t now_us = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
        if (now_us - start_time > timeout_us) return 0; // Timeout
    }

    return end_time - start_time;
}


// Funktion für Busy-Wait in Mikrosekunden
void busy_wait_us(uint32_t us) {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
        uint64_t elapsed_us = ((uint64_t)(now.tv_sec - start.tv_sec)) * 1000000ULL
                             + ((uint64_t)(now.tv_nsec - start.tv_nsec)) / 1000ULL;
        if (elapsed_us >= (uint64_t)us) break;
    } while (true);
}

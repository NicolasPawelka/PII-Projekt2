#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include <applibs/log.h>
#include <applibs/gpio.h>
#include <hw/template_appliance.h>

typedef enum {
    ExitCode_Success = 0,
    ExitCode_Main_Led = 1
} ExitCode;

// Pins
//#define GREEN_LED MT3620_RDB_LED1_GREEN
//#define BLUE_LED  MT3620_RDB_LED2_BLUE
#define TRIG_PIN  MT3620_RDB_HEADER1_PIN4_GPIO
#define ECHO_PIN  MT3620_RDB_HEADER1_PIN6_GPIO


#define RED_LED1 MT3620_RDB_LED1_RED
#define RED_LED2 MT3620_RDB_LED2_RED
#define RED_LED3 MT3620_RDB_LED3_RED
#define RED_LED4 MT3620_RDB_LED4_RED

// Mikrosekunden-Wartefunktion
void busy_wait_us(uint32_t us) {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
        uint64_t elapsed_us = (now.tv_sec - start.tv_sec) * 1000000 + (now.tv_nsec - start.tv_nsec)/1000;
        if(elapsed_us >= us) break;
    } while(true);
}

// Pulsdauer messen
uint64_t measure_pulse_duration(int fdEcho) {
    struct timespec ts;
    uint64_t start_time, end_time;

    // Warte bis Echo HIGH
    while (true) {
        GPIO_Value_Type val;
        GPIO_GetValue(fdEcho, &val);
        if (val == GPIO_Value_High) break;
    }
    clock_gettime(CLOCK_MONOTONIC, &ts);
    start_time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

    // Warte bis Echo LOW
    while (true) {
        GPIO_Value_Type val;
        GPIO_GetValue(fdEcho, &val);
        if (val == GPIO_Value_Low) break;
    }
    clock_gettime(CLOCK_MONOTONIC, &ts);
    end_time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

    return end_time - start_time; // Mikrosekunden
}

int main(void) {
    // LEDs öffnen
   // int fdGreen = GPIO_OpenAsOutput(GREEN_LED, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    //int fdBlue  = GPIO_OpenAsOutput(BLUE_LED, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    int fdRed1  = GPIO_OpenAsOutput(RED_LED1, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    int fdRed2  = GPIO_OpenAsOutput(RED_LED2, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    int fdRed3  = GPIO_OpenAsOutput(RED_LED3, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    int fdRed4  = GPIO_OpenAsOutput(RED_LED4, GPIO_OutputMode_PushPull, GPIO_Value_Low);

    /*if (fdGreen < 0 || fdBlue < 0) {
        Log_Debug("Fehler beim Öffnen der LEDs: %s (%d)\n", strerror(errno), errno);
        return ExitCode_Main_Led;
    }*/

    // Sensor-Pins öffnen
    int fdTrig = GPIO_OpenAsOutput(TRIG_PIN, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    int fdEcho = GPIO_OpenAsInput(ECHO_PIN);
    if (fdTrig < 0 || fdEcho < 0) {
        Log_Debug("Fehler beim Öffnen der Sensor GPIOs: %s (%d)\n", strerror(errno), errno);
        return ExitCode_Main_Led;
    }

    const struct timespec sleepTime = {.tv_sec = 0, .tv_nsec = 200000000}; // 500ms Pause

    while (true) {
        // Trig-Puls 10µs senden
        GPIO_SetValue(fdTrig, GPIO_Value_High);
        busy_wait_us(10);
        GPIO_SetValue(fdTrig, GPIO_Value_Low);

        // Pulsdauer messen
        uint64_t pulse_us = measure_pulse_duration(fdEcho);

        // Entfernung in cm
        float distance_cm = pulse_us / 100;

        Log_Debug("Entfernung: %.1f cm\n", distance_cm);

        // LEDs steuern
        if (distance_cm > 50.0 ) {
            GPIO_SetValue(fdRed1, GPIO_Value_Low);
            GPIO_SetValue(fdRed2,  GPIO_Value_Low);
            GPIO_SetValue(fdRed3, GPIO_Value_Low);
            GPIO_SetValue(fdRed4,  GPIO_Value_Low);
        }
        else if (distance_cm < 40.0){
            GPIO_SetValue(fdRed1, GPIO_Value_High);
            GPIO_SetValue(fdRed2,  GPIO_Value_Low);
            GPIO_SetValue(fdRed3, GPIO_Value_Low);
            GPIO_SetValue(fdRed4,  GPIO_Value_Low);
            

        } 
        else if (distance_cm < 30.0){
            GPIO_SetValue(fdRed1, GPIO_Value_High);
            GPIO_SetValue(fdRed2,  GPIO_Value_High);
            GPIO_SetValue(fdRed3, GPIO_Value_Low);
            GPIO_SetValue(fdRed4,  GPIO_Value_Low);
            

        }
        else if (distance_cm < 20.0){
            GPIO_SetValue(fdRed1, GPIO_Value_High);
            GPIO_SetValue(fdRed2,  GPIO_Value_High);
            GPIO_SetValue(fdRed3, GPIO_Value_High);
            GPIO_SetValue(fdRed4,  GPIO_Value_Low);
            

        }
        else if (distance_cm < 10.0){
            GPIO_SetValue(fdRed1, GPIO_Value_High);
            GPIO_SetValue(fdRed2,  GPIO_Value_High);
            GPIO_SetValue(fdRed3, GPIO_Value_High);
            GPIO_SetValue(fdRed4,  GPIO_Value_High);
            

        }
            
        

        nanosleep(&sleepTime, NULL);
    }

    return ExitCode_Success;
}

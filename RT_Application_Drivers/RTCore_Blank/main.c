#include "FreeRTOS.h"
#include"mt3620.h"
#include "task.h"
#include <stdint.h>
#include "VectorTable.h"
#include "lib/UART.h"
#include "lib/Print.h"
#include "lib/CPUFreq.h"
#include"lib/GPIO.h"
#include "nvic.h"
#include"timers.h"
#include"timer.h"
#include"Common.h"



static UART *debug = NULL;
static UART *driver = NULL;
bool *state = false;


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    for(;;);
}


static void GPIO_Init(){

    GPIO_ConfigurePinForOutput(0);
    GPIO_ConfigurePinForOutput(8);
    GPIO_ConfigurePinForOutput(15);
    GPIO_ConfigurePinForOutput(18);
    GPIO_ConfigurePinForOutput(21);
    
    GPIO_Write(0,false);
    GPIO_Write(8,false);
    GPIO_Write(15,false);
    GPIO_Write(18,false);
    GPIO_Write(21,false);



}

static void measure(){

    GPIO_Write(0,true);
    vTaskDelay(pdMS_TO_TICKS(0.01));
    GPIO_Write(0,false);
    unsigned long long timestamp_1 = get_current_system_us();


    int ret = GPIO_ConfigurePinForInput(0);
    if(ret != ERROR_NONE){
        UART_Print(debug,"Fehler beim umschalten"); 
    }

    while(*state == false){
        GPIO_Read(0,state);
    }
    unsigned long long timestamp_2 = get_current_system_us();

    unsigned long long pulse = timestamp_2 - timestamp_1;

    float distance = pulse / 58.0f;


    UART_Print(debug,"%f");



}



static void gpio_task(void *pParameters)
{
    UART_Print(debug,"Bin in der Task");
    int counter = 0;
    while(1){
        UART_Print(debug,"Tick");
        counter++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

static void uart_tx_task(void *pParameters)
{

}


_Noreturn void RTCoreMain(void){
    
    VectorTableInit();
    CPUFreq_Set(197600000);

    debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    UART_Print(debug, "--------------------------------\r\n");
    UART_Print(debug, "Hallo\r\n");
    UART_Print(debug, "App built on: " __DATE__ " " __TIME__ "\r\n");

    GPIO_Init();


    xTaskCreate(gpio_task, "BLINKI", 256, NULL, 5, NULL);

    vTaskStartScheduler();
    UART_Print(debug, "Scheduler started\r\n");

    for(;;){
       __asm__("wfi");
    }
}
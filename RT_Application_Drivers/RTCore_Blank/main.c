#include "FreeRTOS.h"
#include"mt3620.h"
#include "task.h"
#include <stdint.h>
#include "VectorTable.h"
#include "lib/UART.h"
#include "lib/Print.h"
#include "lib/CPUFreq.h"
#include "nvic.h"
#include"timers.h"



static UART *debug = NULL;
static UART *driver = NULL;


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    for(;;);
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


    xTaskCreate(gpio_task, "BLINKI", 256, NULL, 5, NULL);

    vTaskStartScheduler();
    UART_Print(debug, "Scheduler started\r\n");

    for(;;){
       __asm__("wfi");
    }
}
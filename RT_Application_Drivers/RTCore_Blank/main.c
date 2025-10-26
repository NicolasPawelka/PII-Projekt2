#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include "mt3620.h"
#include "VectorTable.h"
#include "lib/UART.h"
#include "lib/Print.h"
#include "lib/CPUFreq.h"

static UART *debug = NULL;
static UART *driver = NULL;



void _putchar(char character)
{
;
}



/* Wird aufgerufen, wenn ein Task seinen Stack überschreitet */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{

}

/* Hook for "memory allocation failed". */
void vApplicationMallocFailedHook(void)
{

}



static void gpio_task(void *pParameters)
{


}

static void uart_tx_task(void *pParameters)
{

}

_Noreturn void RTCoreMain(void){
    VectorTableInit();
    CPUFreq_Set(26000000);

    debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    //UART_Print(debug, "--------------------------------\r\n");
    UART_Print(debug, "UART_RTApp_MT3620_BareMetal\r\n");
    UART_Print(debug, "App built on: " __DATE__ " " __TIME__ "\r\n");

 

    xTaskCreate(gpio_task, "BLINKI", 512, NULL, 5, NULL);
    vTaskStartScheduler();
    for(;;){
       
    }
}
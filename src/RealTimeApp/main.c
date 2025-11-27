#include "FreeRTOS.h"
#include"mt3620.h"
#include "task.h"
#include <stdint.h>
#include "VectorTable.h"


#include "lib/UART.h"
#include "lib/Print.h"
#include "lib/CPUFreq.h"
#include"lib/GPIO.h"
#include "lib/Platform.h"
#include "lib/I2CMaster.h"

#include "nvic.h"
#include"timers.h"
#include"Common.h"

#include "logical-dpc.h"
#include "logical-intercore.h"

#include "mt3620-baremetal.h"
#include "mt3620-intercore.h"
#include "mt3620-timer.h"

#include"Grove_Shield_Driver/include/GroveI2C.h"
#include"Grove_Shield_Driver/include/GroveUart.h"

#include"motor.h"

static UART *debug = NULL;
static UART *driver_Uart = NULL;
bool *state = false;
TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;


static IntercoreComm icc;
// High-Level-App Component ID
static const ComponentId hlAppId = {
    .data1 = 0xc910d996,
    .data2 = 0xd180,
    .data3 = 0x4702,
    .data4 = {0x8b, 0x85, 0xa6, 0xc7, 0x01, 0x5c, 0x8c, 0x45}
};



#define US_PIN 0
static const uintptr_t GPT_BASE = 0x21030000;



void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    for(;;);
}



// void WriteReg32(uintptr_t baseAddr, size_t offset, uint32_t value){
//     *(volatile uint32_t*)(baseAddr + offset) = value;
// }

// uint32_t ReadReg32(uintptr_t baseAddr, size_t offset)
// {
// 	return *(volatile uint32_t*)(baseAddr + offset);
// }


void Gpt3_WaitUs(int microseconds)
{
	// GPT3_INIT = initial counter value
	WriteReg32(GPT_BASE, 0x54, 0x0);
    

	// GPT3_CTRL
	uint32_t ctrlOn = 0x0;
	ctrlOn |= (0x19) << 16; // OSC_CNT_1US (default value)
	ctrlOn |= 0x1;          // GPT3_EN = 1 -> GPT3 enabled
	WriteReg32(GPT_BASE, 0x50, ctrlOn);

	// GPT3_CNT
	while (ReadReg32(GPT_BASE, 0x58) < microseconds)
	{
		// empty.
	}

	// GPT_CTRL -> disable timer
	WriteReg32(GPT_BASE, 0x50, 0x0);
}


float measure(void)
{
    bool echo = false;
    uint32_t pulseBegin,pulseEnd;

    GPIO_ConfigurePinForOutput(US_PIN);
    GPIO_Write(US_PIN, false);
    Gpt3_WaitUs(2);
    GPIO_Write(US_PIN, true);
    Gpt3_WaitUs(5);
    
    // GPT3_CTRL - starts microsecond resolution clock
	uint32_t ctrlOn = 0x0;
	ctrlOn |= (0x19) << 16; // OSC_CNT_1US (default value)
	ctrlOn |= 0x1;          // GPT3_EN = 1 -> GPT3 enabled
	WriteReg32(GPT_BASE, 0x50, ctrlOn);
    

    GPIO_ConfigurePinForInput(US_PIN);

    

    do {
        GPIO_Read(US_PIN, &echo);
    } while (!echo);

    pulseBegin = ReadReg32(GPT_BASE, 0x58);
    
    do {
        GPIO_Read(US_PIN, &echo);
    } while (echo);

    pulseEnd = ReadReg32(GPT_BASE, 0x58);

	// GPT_CTRL -> disable timer
	WriteReg32(GPT_BASE, 0x50, 0x0);

	return (pulseEnd - pulseBegin) / 58.0;
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


static void gpio_task(void *pParameters)
{
    float dist = 0;
    while(1){

        dist = measure();
        UART_Printf(debug,"Aktuelle Distanz %f\r\n",dist);
        
        if (dist > 30){
            xTaskNotify(task2Handle, 0x01, eSetBits);
        }else{
            xTaskNotify(task2Handle, 0x02 , eSetBits);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}



static void HandleSendTimerDeferred(void)
{
    UART_Print(debug,"Versende Nachricht");
    static char msg[] = "RT->HL";
    IntercoreResult ret =  IntercoreSend(&icc, &hlAppId, msg, sizeof(msg) - 1);

    if (ret == Intercore_OK){
        UART_Print(debug,"Nachrciht gesendet");
    }else{
        UART_Printf(debug,"Exited with code: %x", ret);
    }


}


static void send_task(void *pParameters){

    while(1){
        HandleSendTimerDeferred();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }


}



_Noreturn void RTCoreMain(void){
    
    VectorTableInit();
    CPUFreq_Set(197600000);
    GPIO_Init();

    IntercoreResult icr = SetupIntercoreComm(&icc,NULL);

    if (icr != Intercore_OK){
        UART_Printf(debug,"Error while Intercore Setup: %04x",icr);
    }

    debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    UART_Print(debug, "--------------------------------\r\n");
    UART_Print(debug, "Hallo\r\n");
    UART_Print(debug, "App built on: " __DATE__ " " __TIME__ "\r\n");


    // xTaskCreate(gpio_task, "GPIO_Task",2048,NULL,5,&task1Handle);
    xTaskCreate(motor_task, "Motor_Task", 2048, NULL, 5, &task2Handle);
    xTaskCreate(send_task,"Send_Task",2048,NULL,5,NULL);
    vTaskStartScheduler();


    for(;;){
       __asm__("wfi");
    }
}
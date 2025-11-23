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
#include"timer.h"
#include"Common.h"

#include"Grove_Shield_Driver/include/GroveI2C.h"
#include"Grove_Shield_Driver/include/GroveUart.h"

static UART *debug = NULL;
static UART *driver_Uart = NULL;
bool *state = false;

#define I2C_ADDR_MOTOR_1 0x0f
#define I2C_ADDR_MOTOR_2 0x0d

#define MotorSpeedSet             0x82
#define PWMFrequenceSet           0x84
#define DirectionSet              0xaa
#define MotorSetA                 0xa1
#define MotorSetB                 0xa5
#define Nothing                   0x01

#define I2CMotorDriverAdd         0x1F
#define I2CMotor2                 0x0F

#define CMD_MOTOR_A             1
#define CMD_MOTOR_B             2

static uint8_t rxBuffer[64];
static volatile uint32_t rxWritePos = 0;
uint8_t data;

static void RxHandler()
{
    rxBuffer[rxWritePos++ & 63] = data;
}


void MotorSpeedSetAB(uint8_t var) {
    uint8_t speed = var;

    uint8_t data[3];
    data[0] = MotorSpeedSet;
    data[1] = 0;
    data[2] = speed;         

    int ret = SC18IM700_I2cWrite(driver_Uart, I2CMotor2, data, sizeof(data));

    while(!ret){
        // do nothing
    }

}

void GetRotatedd(uint8_t dir){

    uint8_t data[2];
    data[0] = DirectionSet;
    data[1] = dir;   

    int ret = SC18IM700_I2cWrite(driver_Uart,I2CMotor2,data,sizeof(data));

    while(!ret){
        //do nothing
    }

}



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
    // unsigned long long timestamp_1 = get_current_system_us();


    int ret = GPIO_ConfigurePinForInput(0);
    if(ret != ERROR_NONE){
        UART_Print(debug,"Fehler beim umschalten"); 
    }

    while(*state == false){
        GPIO_Read(0,state);
    }
    // unsigned long long timestamp_2 = get_current_system_us();

    // unsigned long long pulse = timestamp_2 - timestamp_1;

    // float distance = pulse / 58.0f;


    UART_Print(debug,"%f");



}


static void motor_task(void *pParameters)
{
    UART_Print(debug,"Starte Motor Task");
    driver_Uart = UART_Open(MT3620_UNIT_ISU0,9600,UART_PARITY_NONE,1,RxHandler);
    
    while(1){
        MotorSpeedSetAB(50);
        GetRotatedd(0b1010);
        vTaskDelay(pdMS_TO_TICKS(500));
        MotorSpeedSetAB(0);
        vTaskDelay(pdMS_TO_TICKS(500));
        
    }

}

_Noreturn void RTCoreMain(void){
    
    VectorTableInit();
    CPUFreq_Set(197600000);

    debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    UART_Print(debug, "--------------------------------\r\n");
    UART_Print(debug, "Hallo\r\n");
    UART_Print(debug, "App built on: " __DATE__ " " __TIME__ "\r\n");


    
    xTaskCreate(motor_task, "Motor_Task", 512, NULL, 5, NULL);
    vTaskStartScheduler();


    for(;;){
       __asm__("wfi");
    }
}
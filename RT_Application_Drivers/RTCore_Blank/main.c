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




static UART *debug = NULL;
static UART *driver_Uart = NULL;
bool *state = false;

//static UART *driver = NULL;
static I2CMaster *driver   = NULL;
static const uint32_t buttonAGpio = 12;
//static i2c_master_port motor_driver;


#define I2C_ADDR_MOTOR_1 0x0f
#define I2C_ADDR_MOTOR_2 0x0d


// #define CMD_MOTOR_A           0xA1
// #define CMD_MOTOR_B           0xA5

#define CMD_MOTOR_A             1
#define CMD_MOTOR_B             2


void Motor_Set(uint8_t motor, int8_t speed)
{
    uint8_t cmd;
    uint8_t dir;
    uint8_t pwm;

    // motor = I2C_ADDR_MOTOR_1 oder I2C_ADDR_MOTOR_2
    if (motor == I2C_ADDR_MOTOR_1) {
        cmd = CMD_MOTOR_A;
    } else if (motor == I2C_ADDR_MOTOR_2) {
        cmd = CMD_MOTOR_B;
    } else {
        UART_Print(debug, "Ungültiger Motor!\r\n");
        return;
    }

    // Richtung & Geschwindigkeit umrechnen
    if (speed > 0) {
        dir = 0x01; // vorwärts
        pwm = (speed > 100) ? 100 : speed;
    } else if (speed < 0) {
        dir = 0x02; // rückwärts
        pwm = ((-speed) > 100) ? 100 : (-speed);
    } else {
        dir = 0x01; // egal, Geschwindigkeit 0 = stop
        pwm = 0;
    }
     UART_Print(debug, "bis hierhin komme ich\r\n");

    uint8_t data[3] = { cmd, dir, pwm };
    int32_t result = I2CMaster_WriteSync(driver, I2C_ADDR_MOTOR_2 , data, sizeof(data));
    UART_Print(debug, "hier hänge ich ich\r\n");

    if (result != ERROR_NONE) {
        UART_Print(debug, "Fehler beim Senden über I2C!\r\n");
        UART_Printf(debug,"Folgender Fehler aufgestreten: %ld",result);
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

static void motor_task(void *param)
{
    while (1) {
        UART_Print(debug, "Motor A vorwärts\r\n");
        Motor_Set(I2C_ADDR_MOTOR_2, 80);
        vTaskDelay(pdMS_TO_TICKS(2000));

        UART_Print(debug, "Motor A rückwärts\r\n");
        Motor_Set(I2C_ADDR_MOTOR_2, -80);
        vTaskDelay(pdMS_TO_TICKS(2000));

        UART_Print(debug, "Motor A stop\r\n");
        Motor_Set(I2C_ADDR_MOTOR_2, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));



        // UART_Print(debug, "Motor B vorwärts\r\n");
        // Motor_Set(I2C_ADDR_MOTOR_2, 80);
        // vTaskDelay(pdMS_TO_TICKS(2000));

        // UART_Print(debug, "Motor B rückwärts\r\n");
        // Motor_Set(I2C_ADDR_MOTOR_2, -80);
        // vTaskDelay(pdMS_TO_TICKS(2000));

        // UART_Print(debug, "Motor B stop\r\n");
        // Motor_Set(I2C_ADDR_MOTOR_2, 0);
        // vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



_Noreturn void RTCoreMain(void){
    
    VectorTableInit();
    CPUFreq_Set(197600000);

    debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    UART_Print(debug, "--------------------------------\r\n");
    UART_Print(debug, "Hallo\r\n");
    UART_Print(debug, "App built on: " __DATE__ " " __TIME__ "\r\n");
    GPIO_Init();

    int i2cFd;
    
 
    driver = I2CMaster_Open(MT3620_UNIT_ISU3);
    if (!driver) {
        UART_Print(debug,
            "ERROR: I2C initialisation failed\r\n");
    }

    I2CMaster_SetBusSpeed(driver, I2C_BUS_SPEED_STANDARD);
    

    // xTaskCreate(motor_task, "Motor_Task", 512, NULL, 5, NULL);
   xTaskCreate(gpio_task, "BLINKI", 512, NULL, 5, NULL);
    vTaskStartScheduler();

    GPIO_ConfigurePinForInput(buttonAGpio);

    for(;;){
       __asm__("wfi");
    }
}
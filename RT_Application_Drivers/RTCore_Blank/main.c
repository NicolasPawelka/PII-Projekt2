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
TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;

#define I2C_ADDR_MOTOR_1          0x0f
#define I2C_ADDR_MOTOR_2          0x0d

#define MotorSpeedSet             0x82
#define PWMFrequenceSet           0x84
#define DirectionSet              0xaa
#define MotorSetA                 0xa1
#define MotorSetB                 0xa5
#define Nothing                   0x01

#define I2CMotorDriverAdd         0x1F
#define I2CMotor2                 0x0F

#define CMD_MOTOR_A               1
#define CMD_MOTOR_B               2


#define US_PIN 0
static const uintptr_t GPT_BASE = 0x21030000;

static uint8_t rxBuffer[64];
static volatile uint32_t rxWritePos = 0;
uint8_t data;

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    for(;;);
}

static void RxHandler()
{
    rxBuffer[rxWritePos++ & 63] = data;
}

void WriteReg32(uintptr_t baseAddr, size_t offset, uint32_t value){
    *(volatile uint32_t*)(baseAddr + offset) = value;
}

uint32_t ReadReg32(uintptr_t baseAddr, size_t offset)
{
	return *(volatile uint32_t*)(baseAddr + offset);
}


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

void MotorSpeedSetAB(uint8_t motor_1_speed, uint8_t motor_2_speed) {

    uint8_t data_motor_1[3];
    data_motor_1[0] = MotorSpeedSet;
    data_motor_1[1] = motor_1_speed;
    data_motor_1[2] = motor_1_speed; 
    

    uint8_t data_motor_2[3];
    data_motor_2[0] = MotorSpeedSet;
    data_motor_2[1] = motor_2_speed;
    data_motor_2[2] = motor_2_speed;

    int ret1 = SC18IM700_I2cWrite(driver_Uart, I2CMotor2, data_motor_1, sizeof(data_motor_1));
    vTaskDelay(pdMS_TO_TICKS(50));
    int ret2 = SC18IM700_I2cWrite(driver_Uart, I2CMotorDriverAdd, data_motor_2, sizeof(data_motor_2));

    while(!ret1 || !ret2){
        // do nothing
    }

}

float measure(void)
{
    bool echo = false;
    unsigned long long t_start = 0, t_end = 0;
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

void GetRotatedd(uint8_t dir){

    uint8_t data[2];
    data[0] = DirectionSet;
    data[1] = dir;   

    int ret1 = SC18IM700_I2cWrite(driver_Uart,I2CMotor2,data,sizeof(data));
    int ret2 = SC18IM700_I2cWrite(driver_Uart,I2CMotorDriverAdd,data,sizeof(data));
    
    while(!ret1 || !ret2){
        //do nothing
    }

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

static void motor_task(void *pParameters)
{
    UART_Print(debug,"Starte Motor Task");
    // Wichtig UART Baud Rate auf 9600 da das Grove Shield mit diesem Wert bootet
    // Öffnen und Schließen des UART Handles führt zu Problemen daher ist das umgehen
    // der Shield Config sinnvoll
    driver_Uart = UART_Open(MT3620_UNIT_ISU0,9600,UART_PARITY_NONE,1,RxHandler);
    uint32_t notificationValue;
    while(1){
        BaseType_t result = xTaskNotifyWait(0x00, 0x07,&notificationValue, portMAX_DELAY);

        if (result == pdPASS){
            if(notificationValue & 0x01){
                UART_Print(debug,"Sicher zum Fahren");
                MotorSpeedSetAB(25,30);
                GetRotatedd(0b1010);
                vTaskDelay(pdMS_TO_TICKS(500));
            }else if(notificationValue & 0x02){
                MotorSpeedSetAB(0,0);
            }

        }
        
    }

}

_Noreturn void RTCoreMain(void){
    
    VectorTableInit();
    CPUFreq_Set(197600000);

    debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    UART_Print(debug, "--------------------------------\r\n");
    UART_Print(debug, "Hallo\r\n");
    UART_Print(debug, "App built on: " __DATE__ " " __TIME__ "\r\n");


    xTaskCreate(gpio_task, "GPIO_Task",2048,NULL,5,&task1Handle);
    xTaskCreate(motor_task, "Motor_Task", 2048, NULL, 5, &task2Handle);
    vTaskStartScheduler();


    for(;;){
       __asm__("wfi");
    }
}
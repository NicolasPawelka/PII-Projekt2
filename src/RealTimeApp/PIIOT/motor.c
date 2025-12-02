#include"motor.h"


uint8_t rxBuffer[64];
volatile uint32_t rxWritePos;
uint8_t data;

void RxHandler()
{
    rxBuffer[rxWritePos++ & 63] = data;
}


void SetSpeed(UART* driver_Uart,uint8_t motor_1_speed, uint8_t motor_2_speed) {

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


void Drive(UART* driver_Uart, uint8_t motor ,uint8_t dir){

    uint8_t data[2];
    data[0] = DirectionSet;
    data[1] = dir;   

    uint8_t adress = 0;

    if(MOTOR_A == motor){
        adress = I2CMotorDriverAdd;
    }else if(MOTOR_B == motor){
        adress = I2CMotor2;
    }



    int ret1 = SC18IM700_I2cWrite(driver_Uart,adress,data,sizeof(data));
    // int ret2 = SC18IM700_I2cWrite(driver_Uart,I2CMotorDriverAdd,data,sizeof(data));
    
    while(!ret1){
        //do nothing
    }

}




void motor_task(void *pParameters)
{
    // Wichtig UART Baud Rate auf 9600 da das Grove Shield mit diesem Wert bootet
    // Öffnen und Schließen des UART Handles führt zu Problemen daher ist das umgehen
    // der Shield Config sinnvoll
    UART* driver_Uart = UART_Open(MT3620_UNIT_ISU0,9600,UART_PARITY_NONE,1,RxHandler);
    uint32_t notificationValue;
    while(1){
        BaseType_t result = xTaskNotifyWait(0x00, 0x07,&notificationValue, portMAX_DELAY);

        if (result == pdPASS){
            if(notificationValue & 0x01){
                SetSpeed(driver_Uart, 25,30);
                Drive(driver_Uart,MOTOR_A, 0b1010);
                Drive(driver_Uart,MOTOR_B, 0b1010);
                vTaskDelay(pdMS_TO_TICKS(500));
            }else if(notificationValue & 0x02){
                SetSpeed(driver_Uart, 0,0);
                
                //try rotating
                SetSpeed(driver_Uart,25,25);
                Drive(driver_Uart,MOTOR_A,0b1010);
                Drive(driver_Uart,MOTOR_B,0b0101);
                vTaskDelay(pdMS_TO_TICKS(500));
            }

        }
        
    }

}
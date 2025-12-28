#include"motor.h"

#define REG_WHO_AM_I 0x75
#define REG_PWR_MGMT_1  0x6B
#define REG_SMPLRT_DIV  0x19
#define REG_CONFIG      0x1A
#define REG_ACCEL_CFG   0x1C
#define REG_ACCEL_CFG2  0x1D

#define REG_ACCEL_XOUT_H   0x3B
#define REG_ACCEL_XOUT_L   0x3C
#define REG_ACCEL_YOUT_H   0x3D
#define REG_ACCEL_YOUT_L   0x3E
#define REG_ACCEL_ZOUT_H   0x3F
#define REG_ACCEL_ZOUT_L   0x40
#define MPU_ADDR 0xD1


uint8_t rxBuffer[64];
volatile uint32_t rxWritePos;
uint8_t data;
UART* driver_Uart;

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

void MPU9250_Task(void *parameters){

    bool ret_2 = init_MPU9250();
    while(!ret_2){
        
    }
    UART* debug =UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    uint8_t data1;
    uint8_t data2;


    while(1){
        uint8_t h, l;
        int16_t ax;
        bool ret_1 = false;

        ret_1 = GroveI2C_ReadReg8(driver_Uart, MPU_ADDR, REG_ACCEL_XOUT_H, &data1);
        if(ret_1){
            UART_Printf(debug,"Help\r\n");
        }
        // GroveI2C_ReadReg16(driver_Uart, MPU_ADDR, REG_ACCEL_XOUT_L, &data2);

        //SC18IM700_ReadReg(driver_Uart, REG_ACCEL_XOUT_H, &h);
        //SC18IM700_ReadReg(driver_Uart, REG_ACCEL_XOUT_L, &l);
      
        UART_Printf(debug, "Data High: %x\r\n" , data1);
        // UART_Printf(debug, "Data Low: %x\n" , data2);

        vTaskDelay(pdMS_TO_TICKS(500));



    }
}

bool init_MPU9250(){

    driver_Uart = UART_Open(MT3620_UNIT_ISU0,9600,UART_PARITY_NONE,1,RxHandler);
    // uint8_t data = 0xE0;
    // int ret1 = SC18IM700_I2cWrite(driver_Uart, 0xE0, &data, 1);
    // if(!ret1){
    //     while(1){

    //     }
    // }
    
    int done = true;
    uint8_t data_2[2];
    data_2[0] = REG_PWR_MGMT_1;
    data_2[1] = 0x01;   


    int ret_2 = SC18IM700_I2cWrite(driver_Uart, MPU_ADDR,data_2,sizeof(data_2));

    if(!ret_2){
        while(1){

        }
    }   

    data_2[0] = REG_SMPLRT_DIV;
    data_2[1] = 0x05;

    int ret_3 = SC18IM700_I2cWrite(driver_Uart,MPU_ADDR,data_2,sizeof(data_2));

    if(!ret_3){
        while(1){
            
        }
    }

    data_2[0] = REG_CONFIG;
    data_2[1] = 0x03;

    int ret_4 = SC18IM700_I2cWrite(driver_Uart,MPU_ADDR,data_2,sizeof(data_2));

    if(!ret_4){
        while(1){
            
        }
    }

    data_2[0] = REG_ACCEL_CFG;
    data_2[1] = 0x0;

    int ret_5 = SC18IM700_I2cWrite(driver_Uart,MPU_ADDR,data_2,sizeof(data_2));

    if(!ret_5){
        while(1){
            
        }
    }

    data_2[0] = REG_ACCEL_CFG2;
    data_2[1] = 0x06;

    int ret_6 = SC18IM700_I2cWrite(driver_Uart,MPU_ADDR,data_2,sizeof(data_2));

    if(!ret_6){
        while(1){
            
        }
    }
    

    return done;

}





void motor_task(void *pParameters)
{
    // Wichtig UART Baud Rate auf 9600 da das Grove Shield mit diesem Wert bootet
    // Öffnen und Schließen des UART Handles führt zu Problemen daher ist das umgehen
    // der Shield Config sinnvoll
 
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
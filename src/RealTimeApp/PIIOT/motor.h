#include"FreeRTOS.h"
#include<stdint.h>
#include"lib/UART.h"
#include"portmacro.h"
#include"projdefs.h"
#include"task.h"
#include"FreeRTOSConfig.h"
#include"stddef.h"
#include"Grove_Shield_Driver/include/GroveI2C.h"
#include"Grove_Shield_Driver/include/GroveUart.h"

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

void MotorSpeedSetAB(UART*,uint8_t, uint8_t);
void GetRotatedd(UART*,uint8_t);
void motor_task(void *pParameters);
#include <applibs/log.h>
#include <Grove.h>
int i2cFd;


// #define I2C_ADDR_MOTOR_1 0x0F
// #define I2C_ADDR_MOTOR_2 0x1E
// #define MOTOR_SPEED_SET 

// #define CMD_MOTOR_A             0xA1
// #define CMD_MOTOR_B             0xA5

#define MotorSpeedSet             0x82
#define PWMFrequenceSet           0x84
#define DirectionSet              0xaa
#define MotorSetA                 0xa1
#define MotorSetB                 0xa5
#define Nothing                   0x01

#define I2CMotorDriverAdd         0x0f

void Motor_Set(uint8_t motorAddr, int8_t speed)
{
    uint8_t cmd, dir, pwm;

    // Unterscheidung nach Adresse
    if (motorAddr == 0x16 || motorAddr == 0x1E) {
        cmd = 0xA1; // Motor A
    } else if (motorAddr == 0x17 || motorAddr == 0x1F) {
        cmd = 0xA5; // Motor B
    } else {
        Log_Debug("Unbekannte Motoradresse: 0x%02X\r\n", motorAddr);
        return;
    }

    // Richtung & Geschwindigkeit
    if (speed > 0) {
        dir = 0x01;
        pwm = (speed > 100) ? 100 : speed;
    } else if (speed < 0) {
        dir = 0x02;
        pwm = ((-speed) > 100) ? 100 : (-speed);
    } else {
        dir = 0x01;
        pwm = 0;
    }

    uint8_t data[3] = { cmd, dir, pwm };
    GroveI2C_Write(i2cFd, 0x0F, data, 3);
}

void MotorSpeedSetAB(void) {
    uint8_t speed = 100;

    uint8_t data[2];
    data[0] = MotorSpeedSet;
    data[1] = speed;         

    int ret = GroveI2C_Write(i2cFd, I2CMotorDriverAdd, data, sizeof(data));

    if (ret == true) {
        Log_Debug("Success\n");
    } else {
        Log_Debug("Speed Set failed\n");
    }
}

void GetRotatedd(uint8_t dir){

    uint8_t data[2];
    data[0] = DirectionSet;
    data[1] = dir;   
    
    int ret = GroveI2C_Write(i2cFd,I2CMotorDriverAdd,data,sizeof(data));

    if(ret == true){
        Log_Debug("Success");
    }else{
        Log_Debug("Direction Set failed");
    }

}


int main(int argc, char *argv[])
{
    Log_Debug("Application starting\n");
    uint8_t dummy = 0x01;
    int found = 0;

	
	GroveShield_Initialize(&i2cFd, 230400);
    
    // for (uint8_t addr = 1; addr < 0x7F; addr++){

    //     int result = GroveI2C_Write(i2cFd,addr,&dummy,1);

    //     if(result == 1){
    //         Log_Debug("Gerät gefunden bei 0x%02X\n", addr);
    //         found++;
    //     }
    //     usleep(1000);


    // }
    // if (found == 0) {
    //     Log_Debug("Keine I2C-Geräte gefunden.\n");
    // } else {
    //     Log_Debug("Scan abgeschlossen, %d Gerät(e) gefunden.\n", found);
    // }

    // Log_Debug("Fertig.\n");
    // return 0;







    while(1){
        Log_Debug("Starte");
        MotorSpeedSetAB();
        GetRotatedd(0b1010);
        usleep(1000000);
        GetRotatedd(0b0101);
        usleep(1000000);

    }


    Log_Debug("Application exiting\n");
    return 0;
}


#include"motor.h"
#include"ultraschall.h"
#include"general.h"
#include"communication.h"

static UART *debug = NULL;
TaskHandle_t MessTaskHandle = NULL;
TaskHandle_t MotorTaskHandle = NULL;
TaskHandle_t SendeTaskHandle = NULL;
uint8_t rxBuffer_2[64];
volatile uint32_t rxWritePos_2;
uint8_t data_2;

void RxHandler_2()
{
    rxBuffer_2[rxWritePos_2++ & 63] = data_2;
}


_Noreturn void RTCoreMain(void){

    // debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    // UART_Print(debug, "--------------------------------\r\n");
    // UART_Print(debug, "Hallo\r\n");
    // UART_Print(debug, "App built on: " __DATE__ " " __TIME__ "\r\n");
    // UART_Print(debug, "--------------------------------\r\n");

    VectorTableInit();
    CPUFreq_Set(197600000);
    GPIO_Init();
    SetupCommunication();



    

    // xTaskCreate(MPU9250_Task,"Test",4096,NULL,8,NULL);
    xTaskCreate(motor_task, "Task zum Ansteuern der Motor Treiber", 2048, NULL, 6, &MotorTaskHandle);
    xTaskCreate(send_task,"Task zur Kommunikation mit dem A7 Kern (High-Level App)",2048,NULL,5,&SendeTaskHandle);
    xTaskCreate(mess_task, "Task zum kontinuirlichen Messen des Abstands",2048,NULL,8,&MessTaskHandle);
    vTaskStartScheduler();
    // UART* driver_Uart = UART_Open(MT3620_UNIT_ISU0,9600,UART_PARITY_NONE,1,RxHandler_2);
    // uint8_t data = 0xE0;
    // int ret1 = SC18IM700_I2cWrite(driver_Uart, 0xE0, &data, 1);
    // uint8_t dummy[1];
    // dummy[0] = 0x00;

//    for (uint8_t addr = 0x00; addr < 0xFF; addr++) {

//     int ret = SC18IM700_I2cWrite(driver_Uart, addr, dummy, sizeof(dummy));

//     if (ret == true) {
//         UART_Printf(debug, "Found %d\r\n", addr);
//     }
// }


   while(FOREVER){

   }
}
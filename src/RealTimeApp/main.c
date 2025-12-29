#include"motor.h"
#include"ultraschall.h"
#include"general.h"
#include"communication.h"

static UART *debug = NULL;
TaskHandle_t MessTaskHandle = NULL;
TaskHandle_t MotorTaskHandle = NULL;
TaskHandle_t SendeTaskHandle = NULL;


_Noreturn void RTCoreMain(void){

    VectorTableInit();
    CPUFreq_Set(197600000);
    GPIO_Init();
    SetupCommunication();

    xTaskCreate(motor_task, "Task zum Ansteuern der Motor Treiber", 2048, NULL, 6, &MotorTaskHandle);
    xTaskCreate(send_task,"Task zur Kommunikation mit dem A7 Kern (High-Level App)",2048,NULL,5,&SendeTaskHandle);
    xTaskCreate(mess_task, "Task zum kontinuirlichen Messen des Abstands",2048,NULL,8,&MessTaskHandle);
    vTaskStartScheduler();


   while(FOREVER){

   }
}
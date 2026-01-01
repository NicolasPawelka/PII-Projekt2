#include"motor.h"
#include"ultraschall.h"
#include"general.h"
#include"communication.h"
#include"buzzer.h"

TaskHandle_t MessTaskHandle = NULL;
TaskHandle_t MotorTaskHandle = NULL;
TaskHandle_t SendeTaskHandle = NULL;
QueueHandle_t distanceQueue;

_Noreturn void RTCoreMain(void){

    VectorTableInit();
    CPUFreq_Set(197600000);
    GPIO_Init();
    SetupCommunication();

    distanceQueue = xQueueCreate(1, sizeof(float));

    xTaskCreate(motor_task, "Motor", 2048, NULL, 6, &MotorTaskHandle);
    xTaskCreate(send_task, "Send", 2048, NULL, 5, &SendeTaskHandle);
    xTaskCreate(mess_task, "Mess", 2048, NULL, 8, &MessTaskHandle);
    xTaskCreate(buzzer_task, "Buzzer", 2048, NULL, 8, NULL);

    vTaskStartScheduler();



   while(FOREVER){

   }
}
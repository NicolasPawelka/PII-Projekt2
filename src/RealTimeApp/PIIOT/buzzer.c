#include "buzzer.h"
#include "ultraschall.h"
#include"general.h"



void buzzer_task(void *parameter)
{
    float distance = 0.0f;
    TickType_t onTime = 0;
    TickType_t offTime = 0;

    GPIO_ConfigurePinForOutput(BUZZER_PIN);
    GPIO_Write(BUZZER_PIN, false); // Buzzer aus

    while (1)
    {
        /* Warten auf neuen Abstand */
        if (xQueueReceive(distanceQueue, &distance, portMAX_DELAY) == pdTRUE)
        {
            /* Abstand → Frequenz */
            if (distance < 10.0f) {
                onTime  = pdMS_TO_TICKS(50);
                offTime = pdMS_TO_TICKS(50);
            }
            else if (distance < 30.0f) {
                onTime  = pdMS_TO_TICKS(150);
                offTime = pdMS_TO_TICKS(150);
            }
            else if (distance < 50.0f) {
                onTime  = pdMS_TO_TICKS(300);
                offTime = pdMS_TO_TICKS(300);
            }
            else {
                onTime = 0;
                offTime = pdMS_TO_TICKS(200);
            }
        }

        /* Piepsen */
        if (onTime > 0)
        {
            GPIO_Write(BUZZER_PIN, true);
            vTaskDelay(onTime);

            GPIO_Write(BUZZER_PIN, false);
            vTaskDelay(offTime);
        }
        else
        {
            GPIO_Write(BUZZER_PIN, false);
            vTaskDelay(offTime);
        }
    }
}

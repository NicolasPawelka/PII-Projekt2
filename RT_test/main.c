#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include "mt3620.h"


/* Wird aufgerufen, wenn ein Task seinen Stack überschreitet */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    // Optional: Debug-Ausgabe oder LED-Blinken
    taskDISABLE_INTERRUPTS();
    for (;;); // Endlosschleife, damit Fehler sichtbar wird
}

/* Optional: Wird aufgerufen, wenn malloc fehlschlägt */
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;);
}

void BlinkTask(void *params)
{
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    xTaskCreate(BlinkTask, "Blink", 512, NULL, 1, NULL);
    vTaskStartScheduler();

    for (;;); // sollte nie erreicht werden
}

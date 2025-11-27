#include "FreeRTOS.h"
#include"mt3620.h"
#include "task.h"
#include "VectorTable.h"


#include "lib/UART.h"
#include "lib/Print.h"
#include "lib/CPUFreq.h"
#include"lib/GPIO.h"


#define FOREVER 1

void GPIO_Init();
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);

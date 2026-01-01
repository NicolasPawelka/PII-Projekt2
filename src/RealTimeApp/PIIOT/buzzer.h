#ifndef BUZZER_H
#define BUZZER_H

#include "FreeRTOS.h"
#include"mt3620.h"
#include "task.h"
#include "VectorTable.h"


#include "lib/UART.h"
#include "lib/Print.h"
#include "lib/CPUFreq.h"
#include"lib/GPIO.h"

/* Task-Funktion */
void buzzer_task(void *parameter);

#endif

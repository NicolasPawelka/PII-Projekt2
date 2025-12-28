#include"FreeRTOS.h"
#include"task.h"
#include<stdbool.h>
#include"lib/GPIO.h"

#include "mt3620-baremetal.h"
#include "mt3620-intercore.h"
#include "mt3620-timer.h"


#define US_PIN 4
#define BUZZER_PIN 0

void Gpt3_WaitUs(int);
float measure(void);
void mess_task(void*);
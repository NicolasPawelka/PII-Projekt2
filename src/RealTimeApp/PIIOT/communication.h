#include"FreeRTOS.h"
#include"task.h"
#include "logical-dpc.h"
#include "logical-intercore.h"


void HandleSendTimerDeferred(void);
void send_task(void *pParameters);
void SetupCommunication();
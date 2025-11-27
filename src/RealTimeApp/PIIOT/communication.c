#include"communication.h"


static IntercoreComm icc;
// High-Level-App Component ID
static const ComponentId hlAppId = {
    .data1 = 0xc910d996,
    .data2 = 0xd180,
    .data3 = 0x4702,
    .data4 = {0x8b, 0x85, 0xa6, 0xc7, 0x01, 0x5c, 0x8c, 0x45}
};



void HandleSendTimerDeferred(void)
{
    static char msg[] = "RT->HL";
    IntercoreResult ret =  IntercoreSend(&icc, &hlAppId, msg, sizeof(msg) - 1);

    if (ret != Intercore_OK){
        while(true){
            //do nothing
        }
    }


}


void send_task(void *pParameters){

    while(1){
        HandleSendTimerDeferred();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }


}

void SetupCommunication(){
    IntercoreResult icr = SetupIntercoreComm(&icc,NULL);

    if (icr != Intercore_OK){
        while(true){
            //do nothing
        }
    }
}
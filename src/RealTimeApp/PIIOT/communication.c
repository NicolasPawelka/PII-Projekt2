#include"communication.h"
#include"lib/UART.h"
#include"lib/Print.h"
#include"mt3620-uart-poll.h"
extern uint16_t new_value;
static UART* debug;



static IntercoreComm icc;
static const ComponentId hlAppId = {
    .data1 = 0xc910d996,
    .data2 = 0xd180,
    .data3 = 0x4702,
    .data4 = {0x8b, 0x85, 0xa6, 0xc7, 0x01, 0x5c, 0x8c, 0x45}
};



bool SendMessageToA7(void)
{
    
    size_t data_size = sizeof(new_value);
   UART_Printf(debug, "Aktueller Value: %d\r\n", new_value);

    IntercoreResult ret =  IntercoreSend(&icc, &hlAppId, &new_value, data_size);

    if (ret != Intercore_OK){
       return false;
    }

    return true;


}


void send_task(void *pParameters){

    uint32_t notificationValue;
    debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    
    while(1){
        BaseType_t result = xTaskNotifyWait(0x00,0x07, &notificationValue,portMAX_DELAY);
        bool res = false;

        if(result == pdPASS){
            if(notificationValue & 0x01){
                res = SendMessageToA7();
                if (res == false){
                    while (1)
                    {
                        // do nothing
                    }
                    
                }
            }

        }


        
    }
}



static void CallbackForHighLevel(void)
{
    uint8_t tmp[16];
    size_t size = sizeof(tmp);
    ComponentId sender;

    IntercoreResult icr = IntercoreRecv(&icc, &sender, tmp, &size);
    if(icr != Intercore_OK || size == 0) return;

}


void SetupCommunication(){
    IntercoreResult icr = SetupIntercoreComm(&icc,NULL);

    if (icr != Intercore_OK){
        while(true){
            //do nothing
        }
    }
}



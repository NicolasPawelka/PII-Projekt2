#include"Grove_Shield_Driver/include/GroveUart.h"
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

struct UART {
    bool     open;
    unsigned id;
    bool     dma;

    uint32_t txRemain, txRead, txWrite;
    uint32_t rxRemain, rxRead, rxWrite;

    void (*rxCallback)(void);
}; 

void GroveUART_Write(UART* handle,const uint8_t* data, int datasize){

    UART_Write(handle,data,datasize);    
    
}

bool GroveUART_Read(UART* handle,uint8_t* data, int datasize){

    int ret = UART_Read(handle,data,datasize);

    if(ret != ERROR_NONE){
        return false;
    }

    return true;
}




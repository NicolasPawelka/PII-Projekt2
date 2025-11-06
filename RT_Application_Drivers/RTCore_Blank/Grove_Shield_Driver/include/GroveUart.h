#include<stdint.h>
#include<stdbool.h>
#include"lib/UART.h"



void GroveUART_Write(UART* handle,const uint8_t* data, int datasize);
bool GroveUART_Read(UART* handle,uint8_t* data, int datasize);
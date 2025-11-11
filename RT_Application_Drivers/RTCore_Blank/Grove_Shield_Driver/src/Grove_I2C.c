#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include"lib/UART.h"
#include"Grove_Shield_Driver/include/GroveI2C.h"
#include "FreeRTOS.h"
#include"task.h"
#include"mt3620.h"
#include"lib/mt3620/uart.h"
#include"Print.h"

/**
	Set bauud rate for SC18IM700
*/
const uint8_t baudrate_230400_conf[4] = { 0x00, 0x10, 0x01, 0x00 };
const uint8_t baudrate_115200_conf[4] = { 0x00, 0x30, 0x01, 0x00};
const uint8_t baudrate_19200_conf[4] = { 0x00, 0x70, 0x01, 0x01};
const uint8_t baudrate_14400_conf[4] = { 0x00, 0xF4, 0x01, 0x01};
const uint8_t baudrate_9600_conf[4] = { 0x00, 0xF0, 0x01, 0x02};


struct UART {
    bool     open;
    unsigned id;
    bool     dma;

    uint32_t txRemain, txRead, txWrite;
    uint32_t rxRemain, rxRead, rxWrite;

    void (*rxCallback)(void);
}; 


static void wait_for_i2cState_ok(UART* handle){

    uint8_t state;
    SC18IM700_ReadReg(handle,0x0A, &state);

    while(state != 0xF0){
        SC18IM700_ReadReg(handle,0x0A,&state);
    }


}



static void SC18IM700_I2cWrite(UART* handle, uint8_t address, const uint8_t* data, int dataSize)
{	
	// Send
	uint8_t send[3 + dataSize + 1];

	send[0] = 'S';
	send[1] = address & 0xfe;
	send[2] = (uint8_t)dataSize;
	memcpy(&send[3], data, (size_t)dataSize);
	send[3 + dataSize] = 'P';

	
    UART_Write(handle,send, (int)sizeof(send));

	wait_for_i2cState_ok(handle);
}

static bool SC18IM700_I2cRead(UART* handle, uint8_t address, uint8_t* data, int dataSize)
{
	// Send

	uint8_t send[4];

	send[0] = 'S';
	send[1] = address | 0x01;
	send[2] = (uint8_t)dataSize;
	send[3] = 'P';

    if (!UART_Write(handle,send,sizeof(send))){
        return false;
    }

	// Receive

	if (UART_Read(handle, data, dataSize) != ERROR_NONE) return false;

	return true;
}

bool SC18IM700_ReadReg(UART* handle, uint8_t reg, uint8_t* data)
{
	// Send

	uint8_t send[3];
	UART* debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);

	send[0] = 'R';
	send[1] = reg;
	send[2] = 'P';
	
    UART_Write(handle,send,3);
// uint8_t rx;
// const uint8_t cmd[] = { 'R', 0x00, 'P' };

// UART_Write(handle, cmd, sizeof(cmd));

  while(!UART_IsWriteComplete(handle)){
        
    }

// for (int i = 0; i < 1000000; ++i) {
//     if (MT3620_UART_FIELD_READ(handle->id, lsr, dr)) {
//         rx = (uint8_t)mt3620_uart[handle->id]->rbr;
//         UART_Printf(debug,"Empfangenes Byte: 0x%02X\r\n", rx);
//         break;
//     }
// }
  

	// Receive

    if(UART_Read(handle,data,1) != ERROR_NONE){
        return false;
    }
    

	return true;
}


void SC18IM700_WriteReg(UART* handle, uint8_t reg, uint8_t data)
{
	// Send

	uint8_t send[4];

	send[0] = 'W';
	send[1] = reg;
	send[2] = data;
	send[3] = 'P';


    UART_Write(handle,send,sizeof(send));
}



void SC18IM700_WriteRegBytes(UART* handle, uint8_t *data, uint8_t dataSize)
{
	// Send

	uint8_t send[2 + dataSize];

	send[0] = 'W';
	memcpy(&send[1], data, (uint8_t)dataSize);
	send[dataSize+1] = 'P';

	UART_Write(handle,send,(uint8_t)(dataSize + 2));
    
}

void(*GroveI2C_Write)(UART* handle, uint8_t address, const uint8_t* data, int dataSize) = SC18IM700_I2cWrite;
bool(*GroveI2C_Read)(UART* handle, uint8_t address, uint8_t* data, int dataSize) = SC18IM700_I2cRead;


void GroveI2C_WriteReg8(UART* handle, uint8_t address, uint8_t reg, uint8_t val)
{
	uint8_t send[2];
	send[0] = reg;
	send[1] = val;
	GroveI2C_Write(handle, address, send, sizeof(send));
}

void GroveI2C_WriteBytes(UART* handle, uint8_t address, uint8_t *data, uint8_t dataSize)
{
	uint8_t send[dataSize];
	memcpy(send, data, dataSize);

	GroveI2C_Write(handle, address, send, (int)sizeof(send));
}

bool GroveI2C_ReadReg8(UART* handle, uint8_t address, uint8_t reg, uint8_t* val)
{
	GroveI2C_Write(handle, address, &reg, 1);

	uint8_t recv[1];
	if (GroveI2C_Read(handle, address, recv, sizeof(recv)) != ERROR_NONE) return false;

	*val = recv[0];

	return true;
}

bool GroveI2C_ReadReg16(UART* handle, uint8_t address, uint8_t reg, uint16_t* val)
{
	GroveI2C_Write(handle, address, &reg, 1);

	uint8_t recv[2];
	if (GroveI2C_Read(handle, address, recv, sizeof(recv)) != ERROR_NONE) return false;

	*val = (uint16_t)(recv[1] << 8 | recv[0]);

	return true;
}

bool GroveI2C_ReadReg24BE(UART* handle, uint8_t address, uint8_t reg, uint32_t* val)
{
	GroveI2C_Write(handle, address, &reg, 1);

	uint8_t recv[3];
	if (GroveI2C_Read(handle, address, recv, sizeof(recv)) != ERROR_NONE) return false;

	*val = (uint32_t)(recv[0] << 16 | recv[1] << 8 | recv[2]);

	return true;
}




static void baudrate_conf(UART* handle, unsigned int baudrate)
{
	static uint8_t trial = 0;
	uint8_t d0, d1;
	uint8_t conf[4] = { 0 };

    UART_Close(handle);
	handle = UART_Open(MT3620_UNIT_ISU0, 9600,UART_PARITY_NONE,1,NULL);

	while (true)
	{
		d0 = 0, d1 = 0;
		while (d0 != baudrate_9600_conf[1])
		{
			SC18IM700_ReadReg(handle, 0x00, &d0);		
		}
		while (d1 != baudrate_9600_conf[3])
		{
			SC18IM700_ReadReg(handle, 0x01, &d1);
		}		
		//Log_Debug("BR1: %x %x\n", d0, d1);
		if (d0 == baudrate_9600_conf[1] && d1 == baudrate_9600_conf[3]) break;
		vTaskDelay(pdMS_TO_TICKS(5));
	}

	/** Change UART baudrate for SC18IM700 */		
	if (baudrate == 230400) memcpy(conf, baudrate_230400_conf, 4);
	else if(baudrate == 115200) memcpy(conf, baudrate_115200_conf, 4);
	else if (baudrate == 19200) memcpy(conf, baudrate_19200_conf, 4);
	else if (baudrate == 14400) memcpy(conf, baudrate_14400_conf, 4);
	else if (baudrate == 9600) memcpy(conf, baudrate_9600_conf, 4);
	else {
		return;
	}
	SC18IM700_WriteRegBytes(handle, conf, 4);

	UART_Close(handle);
	handle = UART_Open(MT3620_UNIT_ISU0, baudrate,UART_PARITY_NONE,1,NULL);

	SC18IM700_ReadReg(handle, 0x00, &d0);
	SC18IM700_ReadReg(handle, 0x01, &d1);

	//Log_Debug("BR2: %x %x\n", d0, d1);

	if ((d0 != conf[1]) || (d1 != conf[3]))
	{
		trial++;
		if (trial > 10) return;
		baudrate_conf(handle, baudrate);
	}
	trial = 0;
}
void GroveShield_Initialize(UART* handle, uint32_t baudrate)
{
	baudrate_conf(handle, baudrate);
}
#include <stdint.h>
#include <stdbool.h>
#include"lib/UART.h"

#define I2C_OK								0xF0
#define I2C_NACK_ON_ADDRESS		0xF1
#define I2C_NACK_ON_DATA			0xF2
#define I2C_TIME_OUT					0xF8

bool SC18IM700_ReadReg(UART* handle, uint8_t reg, uint8_t* data);
void SC18IM700_WriteReg(UART* handle, uint8_t reg, uint8_t data);
void SC18IM700_WriteRegBytes(UART* handle, uint8_t *data, uint8_t dataSize);
bool SC18IM700_I2cWrite(UART* handle, uint8_t address, const uint8_t* data, int dataSize);


void GroveI2C_WriteReg8(UART* handle, uint8_t address, uint8_t reg, uint8_t val);
void GroveI2C_WriteBytes(UART* handle, uint8_t address, uint8_t *data, uint8_t dataSize);

bool GroveI2C_ReadReg8(UART* handle, uint8_t address, uint8_t reg, uint8_t* val);
bool GroveI2C_ReadReg16(UART* handle, uint8_t address, uint8_t reg, uint16_t* val);
bool GroveI2C_ReadReg24BE(UART* handle, uint8_t address, uint8_t reg, uint32_t* val);

void GroveShield_Initialize(UART* handle, uint32_t baudrate);
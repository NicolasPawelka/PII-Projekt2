#include"ultraschall.h"
#include"general.h"


static const uintptr_t GPT_BASE = 0x21030000;
uint16_t new_value;
extern TaskHandle_t MessTaskHandle;
extern TaskHandle_t MotorTaskHandle;
extern TaskHandle_t SendeTaskHandle;


// void WriteReg32(uintptr_t baseAddr, size_t offset, uint32_t value){
//     *(volatile uint32_t*)(baseAddr + offset) = value;
// }

// uint32_t ReadReg32(uintptr_t baseAddr, size_t offset)
// {
// 	return *(volatile uint32_t*)(baseAddr + offset);
// }

float measure(void)
{
    bool echo = false;
    uint32_t pulseBegin,pulseEnd;

    GPIO_ConfigurePinForOutput(US_PIN);
    GPIO_Write(US_PIN, false);
    Gpt3_WaitUs(2);
    GPIO_Write(US_PIN, true);
    Gpt3_WaitUs(5);
    
    // GPT3_CTRL - starts microsecond resolution clock
	uint32_t ctrlOn = 0x0;
	ctrlOn |= (0x19) << 16; // OSC_CNT_1US (default value)
	ctrlOn |= 0x1;          // GPT3_EN = 1 -> GPT3 enabled
	WriteReg32(GPT_BASE, 0x50, ctrlOn);
    

    GPIO_ConfigurePinForInput(US_PIN);

    

    do {
        GPIO_Read(US_PIN, &echo);
    } while (!echo);

    pulseBegin = ReadReg32(GPT_BASE, 0x58);
    
    do {
        GPIO_Read(US_PIN, &echo);
    } while (echo);

    pulseEnd = ReadReg32(GPT_BASE, 0x58);

	// GPT_CTRL -> disable timer
	WriteReg32(GPT_BASE, 0x50, 0x0);

	return (pulseEnd - pulseBegin) / 58.0;
}


void Gpt3_WaitUs(int microseconds)
{
	// GPT3_INIT = initial counter value
	WriteReg32(GPT_BASE, 0x54, 0x0);
    

	// GPT3_CTRL
	uint32_t ctrlOn = 0x0;
	ctrlOn |= (0x19) << 16; // OSC_CNT_1US (default value)
	ctrlOn |= 0x1;          // GPT3_EN = 1 -> GPT3 enabled
	WriteReg32(GPT_BASE, 0x50, ctrlOn);

	// GPT3_CNT
	while (ReadReg32(GPT_BASE, 0x58) < microseconds)
	{
		// empty.
	}

	// GPT_CTRL -> disable timer
	WriteReg32(GPT_BASE, 0x50, 0x0);
}



void mess_task(void* parameter)
{
    // UART* debug = UART_Open(MT3620_UNIT_UART_DEBUG,115200,UART_PARITY_NONE,1,NULL);
    
    float dist = 0;
    GPIO_ConfigurePinForOutput(BUZZER_PIN);
    while(1){

        dist = measure();
        // UART_Printf(debug,"Aktueller Value: %f",dist);
        new_value = dist;
        
        if (dist > 30){
            //xTaskNotify(MotorTaskHandle, 0x01, eSetBits);
             xTaskNotify(SendeTaskHandle, 0x01, eSetBits);
        }else{
            //xTaskNotify(MotorTaskHandle, 0x02 , eSetBits);
            xTaskNotify(SendeTaskHandle, 0x01, eSetBits);
            GPIO_Write(BUZZER_PIN,true);
            vTaskDelay(pdMS_TO_TICKS(500));
            GPIO_Write(BUZZER_PIN,false);        
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }

}
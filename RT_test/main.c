#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include "mt3620.h"
#include "os_hal_uart.h"
#include"os_hal_gpio.h"
#include"os_hal_dma.h"
#include "printf.h"

static const UART_PORT uart_port_num = OS_HAL_UART_ISU0;
static const mhal_uart_data_len uart_data_len = UART_DATA_8_BITS;
static const mhal_uart_parity uart_parity = UART_NONE_PARITY;
static const mhal_uart_stop_bit uart_stop_bit = UART_STOP_1_BIT;
static const uint32_t uart_baudrate = 115200;

static const os_hal_gpio_pin gpio_led_red = OS_HAL_GPIO_8;
static const os_hal_gpio_pin gpio_led_green = OS_HAL_GPIO_9;
static const os_hal_gpio_pin gpio_button_a = OS_HAL_GPIO_12;
static const os_hal_gpio_pin gpio_button_b = OS_HAL_GPIO_13;

void _putchar(char character)
{
	mtk_os_hal_uart_put_char(uart_port_num, character);
	if (character == '\n')
		mtk_os_hal_uart_put_char(uart_port_num, '\r');
}



/* Wird aufgerufen, wenn ein Task seinen Stack überschreitet */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	printf("%s: %s\n", __func__, pcTaskName);
}

/* Hook for "memory allocation failed". */
void vApplicationMallocFailedHook(void)
{
	printf("%s\n", __func__);
}



static void gpio_task(void *pParameters)
{
	os_hal_gpio_data value = 0;

	printf("GPIO Task Started\n");

	/* Init GPIO */
	mtk_os_hal_gpio_set_direction(gpio_led_red, OS_HAL_GPIO_DIR_OUTPUT);
	mtk_os_hal_gpio_set_direction(gpio_led_green, OS_HAL_GPIO_DIR_OUTPUT);
	mtk_os_hal_gpio_set_direction(gpio_button_a, OS_HAL_GPIO_DIR_INPUT);
	mtk_os_hal_gpio_set_direction(gpio_button_b, OS_HAL_GPIO_DIR_INPUT);

	while (1) {
		/* Get Button_A status and set LED Red. */
		mtk_os_hal_gpio_get_input(gpio_button_a, &value);
		if (value == OS_HAL_GPIO_DATA_HIGH)
			mtk_os_hal_gpio_set_output(gpio_led_red, OS_HAL_GPIO_DATA_HIGH);
		else
			mtk_os_hal_gpio_set_output(gpio_led_red, OS_HAL_GPIO_DATA_LOW);

		/* Get Button_B status and set LED Green. */
		mtk_os_hal_gpio_get_input(gpio_button_b, &value);
		if (value == OS_HAL_GPIO_DATA_HIGH)
			mtk_os_hal_gpio_set_output(gpio_led_green, OS_HAL_GPIO_DATA_LOW);
		else
			mtk_os_hal_gpio_set_output(gpio_led_green, OS_HAL_GPIO_DATA_HIGH);

		/* Delay for 100ms */
		vTaskDelay(pdMS_TO_TICKS(100));
	}

}

static void uart_tx_task(void *pParameters)
{
	#define DMA_BUF_SIZE 64
	uint8_t counter = 0;
	int32_t ret = 0;
	char *dma_buf = NULL;

	dma_buf = pvPortMalloc(DMA_BUF_SIZE);
	printf("UART Tx task started, print log for every second.\n");
	while (1) {
		/* Delay 1000ms */
		vTaskDelay(pdMS_TO_TICKS(1000));

		/* print log to UART by printf, _putchar() will be invoked. */
		printf("UART Tx(printf) Counter ... %d\n", counter++);

		/* Delay 1000ms */
		vTaskDelay(pdMS_TO_TICKS(1000));

		/* print log to UART by DMA, _putchar will not be invoked. */
		memset(dma_buf, 0, DMA_BUF_SIZE);
		snprintf(dma_buf, DMA_BUF_SIZE, "\rUART Tx(DMA)    Counter ... %d\r\n", counter++);
		ret = mtk_os_hal_uart_dma_send_data(uart_port_num, (u8 *)dma_buf, DMA_BUF_SIZE, 0, 100);
		if (ret < 0)
			printf("UART Tx(DMA) Error! (%ld)\n", ret);
		else if (ret != DMA_BUF_SIZE)
			printf("UART Tx(DMA) not completed! Byte transferred:%ld)\n", ret);
	}
}

_Noreturn void RTCoreMain(void){
    NVIC_SetupVectorTable();

    
    mtk_os_hal_uart_ctlr_init(uart_port_num);
    mtk_os_hal_uart_set_format(uart_port_num,uart_data_len,uart_parity,uart_stop_bit);
    mtk_os_hal_uart_set_baudrate(uart_port_num,uart_baudrate);
    printf("\nUART Demo\n");
    xTaskCreate(gpio_task, "BLINKI", 512, NULL, 5, NULL);
    vTaskStartScheduler();
    for(;;){
       
    }
}
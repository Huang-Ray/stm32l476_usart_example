
#include "stm32l4xx.h"              /*Device header*/
#include "gpio_hori.h"

/* user define */
#define CMD_BEF_LEN (100)

/* Baudrate */
#define USART_BAUDRATE          115200

/* USART clock */
#define USART_CLK             20000000

/* Define  Baudrate setting (BRR) for USART */
#define __DIV(__PCLK, __BAUD)        ((__PCLK * 25) / (4 * __BAUD))
#define __DIVMANT(__PCLK, __BAUD)    (__DIV(__PCLK, __BAUD) / 100)
#define __DIVFRAQ(__PCLK, __BAUD)    (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD)  ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))


void SystemCoreClockConfigure(void);
void Delay (uint32_t dlyTicks);
void SysTick_Handler(void);

void usart2_init(void);
void UART2_SendChar (char c);
void UART2_SendString (char *string);
uint8_t UART2_GetChar(void);
void UART2_GetString(char *strBuffer, int bufferSize);

void USART2_IRQHandler(void);

extern volatile uint32_t msTicks;                            /* counts 1ms timeTicks */
extern volatile char cmd;

void delay (uint32_t time);
	
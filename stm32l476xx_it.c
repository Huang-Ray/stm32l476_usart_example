#include "stm32l476xx_it.h"
#include "main.h"


void SysTick_Handler(void) {
  msTicks++;
}


void USART2_IRQHandler(void)
{
	if (USART2->ISR & (1 << 5))
	{
		cmd = (char)UART2_GetChar();
	}
	UART2_SendString("Get data : ");
	UART2_SendString(&cmd);
	
	if (USART2->ISR & (0x1 << 3))
	{
		USART2->ICR |= (0x1 << 3);
	}
}

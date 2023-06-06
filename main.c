

/*#include "stm32l4xx.h"*/              /*Device header*/
#include "main.h"
#include <stdio.h>

volatile uint32_t msTicks;
volatile uint32_t flag;
volatile char debug;
volatile char cmd;
char cmd_str[CMD_BEF_LEN];


void delay (uint32_t time)
{
	while (time--);
}

void UART2_SendChar (char c)
{
	/*********** STEPS FOLLOWED *************
	
	1. Write the data to send in the USART_DR register (this clears the TXE bit). Repeat this
		 for each data to be transmitted in case of single buffer.
	2. After writing the last data into the USART_DR register, wait until TC=1. This indicates
		 that the transmission of the last frame is complete. This is required for instance when
		 the USART is disabled or enters the Halt mode to avoid corrupting the last transmission.
	
	****************************************/

	USART2->TDR = c;   /* Load the Data */
	while (!(USART2->ISR & (1 << 6)));  /*  Wait for TC to SET.. This indicates that the data has been transmitted */
}

void UART2_SendString (char *string)
{
	while (*string)
	{
		UART2_SendChar (*string++);
	}
}


uint8_t UART2_GetChar(void)
{
	uint8_t temp_ch;
	
	while (!(USART2->ISR & (1 << 5)));
	
	temp_ch = (uint8_t)USART2->RDR;
	
	return temp_ch;
}

void UART2_GetString(char *strBuffer, int bufferSize)
{
	int i = 0;
	char temp_ch;
		
	while (1)
	{
		temp_ch = (char)UART2_GetChar(); 
		if ((temp_ch == '\n') || (temp_ch == '\r'))
		{
			*(strBuffer + i) = '\0';
			break;
		}
		
		if (i < bufferSize - 1)
		{
			*(strBuffer + i) = temp_ch;
			++i;
		}
	}
}


int main(void)
{
	SystemCoreClockConfigure();
	SystemCoreClockUpdate();                      /* should update system clock, if never do this, system maybe error*/
	SysTick_Config(SystemCoreClock / 1000);        /* SysTick 1 msec interrupts */

	usart2_init();
	
	UART2_SendString("Hi, i am stm32l476rg \n");
	delay (100);
	
	while (1)
	{

	}
}

/* clock tree set*/
void SystemCoreClockConfigure(void)
{
	RCC->CR |= ((uint32_t)RCC_CR_HSION); /* enable HSI clock (CR bit8: HSION)*/
	while ((RCC->CR & RCC_CR_HSIRDY) == 0); /* wait HSI clock ready (CR bit10: HSIRDY)*/
	
	RCC->CFGR = RCC_CFGR_SW_HSI;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI); /* wait for HSI used as system clock */
		                                                      /* wait sws[1:0] change to 01*/
	
	/* Adaptive real-time memory accelerator (ART Accelerator) */
	/* Incressing process speed by enable instruction prefetch, instruction cache memory and data management*/
	FLASH->ACR = (FLASH_ACR_PRFTEN |         /* Enable Prefetch Buffer */
	              FLASH_ACR_ICEN   |         /* Instruction cache enable */
	              FLASH_ACR_DCEN   |         /* Data cache enable */
	              FLASH_ACR_LATENCY_4WS);    /* Flash 4 wait state */
	
	RCC->CFGR |= (RCC_CFGR_HPRE_DIV1  |      /* HCLK = SYSCLK (AHB Prescaler = 1)*/
	              RCC_CFGR_PPRE1_DIV2 |      /* APB1 = HCLK/2 (ABP1 Prescaler = 2)*/
	              RCC_CFGR_PPRE2_DIV1  );    /* APB2 = HCLK/1 (ABP2 Prescaler = 1)*/
	
	RCC->CR &= ~ RCC_CR_PLLON;   /* PLL off (if you want to set PLLCFG, must at PLL_OFF) */
	
	/* PLL configuration:  VCO = HSI/M * N,  Sysclk = VCO/R */
	RCC->PLLCFGR = ( 0ul << 4 |               /* PLL_M = 1 */
	               (10ul << 8)|               /* PLL_N =  10 */
	               (RCC_PLLCFGR_PLLSRC_HSI) | /* PLL_SRC = HSI */
								 (  1ul << 21) |            /* PLL_Q =   4 */
	               (RCC_PLLCFGR_PLLREN)     |
								 (  1ul << 25));            /* PLL_R =   4 */
								 
  RCC->CR |= RCC_CR_PLLON;
	while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();  /* Wait till PLL is ready */

	RCC->CFGR &= ~RCC_CFGR_SW;                      /* Select PLL as system clock source */
  RCC->CFGR |=  RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  /* Wait till PLL is system clock src */
	
	/*
		SYSCLK = 40MHz
		HCLK   = 40MHz
		PCLK1  = 20MHz
		PCLK2  = 40MHz
	
	*/
}

void usart2_init(void)
{
	  /* Enable GPIOA clock */
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
		while ((RCC->AHB2ENR & RCC_AHB2ENR_GPIOAEN_Msk) != RCC_AHB2ENR_GPIOAEN);  /* wait GPIOA clock enable*/

		RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
		while((RCC->APB1ENR1 & RCC_APB1ENR1_USART2EN_Msk) != RCC_APB1ENR1_USART2EN); /* wait USART2 clock enable */
	
	  /* Set PA2 to alternate function */
	  GPIOA->MODER &= ~GPIO_MODER_MODE2_Msk;
	  GPIOA->MODER |= GPIO_MODER_MODER2_1;
	  /* Set PA3 to alternate function */
	  GPIOA->MODER &= ~GPIO_MODER_MODE3_Msk;
	  GPIOA->MODER |= GPIO_MODER_MODER3_1;

	  /* choose function of PA2, it is AF7 (usart2_tx) here. */
		GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL2_Msk;
	  GPIOA->AFR[0] |= (0x7 << GPIO_AFRL_AFSEL2_Pos);
		/* choose function of PA3, it is AF7 (usart2_rx) here. */
		GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL3_Msk;
	  GPIOA->AFR[0] |= (0x7 << GPIO_AFRL_AFSEL3_Pos);
	
	  USART2->CR1 = 0x00;     /* clear CR register*/
	  USART2->BRR = __USART_BRR(USART_CLK, USART_BAUDRATE);
		USART2->CR3 = 0x0000;  /* no flow control */
		USART2->CR2 = 0x0000;  /* 1 stop bit */
		USART2->CR1 |= USART_CR1_RE;  /* Rx enable */
		USART2->CR1 |= USART_CR1_TE;  /* Tx enable */
		USART2->CR1 |= 0ul << 12;     /* 1 start bit, 8 data bits */
		USART2->CR1 |= USART_CR1_RXNEIE; /* RXNE interrupt enable */
	  USART2->CR1 |= USART_CR1_UE;  /* Enable usart */
		
		NVIC_EnableIRQ(USART2_IRQn);
}

void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) { __NOP(); }
}

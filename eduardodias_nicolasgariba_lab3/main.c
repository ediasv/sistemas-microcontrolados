// main.c
// Desenvolvido para a placa EK-TM4C1294XL
// Lab 3 - Controle bidirecional de velocidade do motor DC
// Autores: Eduardo Dias e Nicolas Gariba

#include <stdint.h>

void PLL_Init(void);
void SysTick_Init(void);
void GPIO_Init(void);
void Timers_Init(void);
void UART_Init(void);
void UART_TxString(const char *s);
void UART_TxChar(char c);              //TESTE-BAUD
void SysTick_Wait1ms(uint32_t ms);     //TESTE-BAUD

int main(void)
{
	PLL_Init();
	SysTick_Init();
	GPIO_Init();
	Timers_Init();
	UART_Init();

	UART_TxString("Motor parado, pressione '*' para iniciar.\r\n");

	while (1) //laco principal
	{
		UART_TxString("Motor parado, pressione '*' para iniciar.\r\n"); //TESTE-BAUD: remover
		SysTick_Wait1ms(500);   //TESTE-BAUD: remover
	}
}

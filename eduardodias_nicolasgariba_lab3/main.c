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
void SysTick_Wait1ms(uint32_t ms);

int main(void)
{
	PLL_Init();
	SysTick_Init();
	GPIO_Init();
	Timers_Init();
	UART_Init();

	//Espera apos o reset antes da primeira mensagem; sem ela o terminal recebe lixo
	SysTick_Wait1ms(2000);

	UART_TxString("Motor parado, pressione '*' para iniciar.\r\n");

	while (1) //laco principal
	{
	}
}

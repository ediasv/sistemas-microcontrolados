// main.c
// Desenvolvido para a placa EK-TM4C1294XL
// Lab 3 - Controle bidirecional de velocidade do motor DC
// Autores: Eduardo Dias e Nicolas Gariba

#include <stdint.h>

void PLL_Init(void);
void SysTick_Init(void);
void GPIO_Init(void);
void Timers_Init(void);

int main(void)
{
	PLL_Init();
	SysTick_Init();
	GPIO_Init();
	Timers_Init();

	while (1) //laco principal
	{
	}
}

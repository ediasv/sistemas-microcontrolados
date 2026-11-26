// timer.c
// Desenvolvido para a placa EK-TM4C1294XL
// Timer0A periodico de 1ms usado como base de tempo

#include <stdint.h>

#include "tm4c1294ncpdt.h"

//Tempo = (1/sysclk)*(contagem+1)*(prescale+1)
//1ms = (1/80MHz)*(contagem+1) -> contagem = 79999 (32 bits, sem prescale)
#define CONTAGEM_1MS 79999

volatile uint32_t tempo_ms = 0; //ms desde a inicializacao (atualizado somente pela ISR)

// -------------------------------------------------------------------------------
// Funcao Timers_Init
// Configura o Timer0A no modo periodico, gerando uma interrupcao a cada 1ms
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void Timers_Init(void)
{
	//1. Habilitar o timer0 no RCGCTIMER e esperar estar pronto no PRTIMER
	SYSCTL_RCGCTIMER_R |= 0x01;
	while((SYSCTL_PRTIMER_R & 0x01) != 0x01){};

	//2. Desabilitar o timer antes de fazer as alteracoes (TAEN = 0)
	TIMER0_CTL_R &= ~0x01;

	//3. Modo 32 bits
	TIMER0_CFG_R = 0x00;

	//4. Modo periodico
	TIMER0_TAMR_R = 0x02;

	//5. Valor de contagem
	TIMER0_TAILR_R = CONTAGEM_1MS;

	//6. Sem prescale
	TIMER0_TAPR_R = 0x00;

	//7. Limpar o flag de interrupcao (ACK)
	TIMER0_ICR_R = 0x01;

	//8a. Habilitar a interrupcao de time-out do timerA
	TIMER0_IMR_R = 0x01;

	//8b. Prioridade da interrupcao no NVIC: Timer0A e a interrupcao 19,
	//    campo dos bits 31-29 do PRI4
	NVIC_PRI4_R = 4 << 29;

	//8c. Habilitar a interrupcao 19 no NVIC
	NVIC_EN0_R = 1 << 19;

	//9. Habilitar o timer (TAEN = 1)
	TIMER0_CTL_R |= 0x01;
}

// -------------------------------------------------------------------------------
// Funcao Timer0A_Handler
// Rotina de tratamento da interrupcao do Timer0A (a cada 1ms)
void Timer0A_Handler(void)
{
	//Limpar o flag de interrupcao (ACK)
	TIMER0_ICR_R = 0x01;
	tempo_ms++;
}

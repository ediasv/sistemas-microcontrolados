// timer.c
// Desenvolvido para a placa EK-TM4C1294XL
// Timer0A periodico de 1ms usado como base de tempo do jogo

#include <stdint.h>

#include "tm4c1294ncpdt.h"

//Tempo = (1/sysclk)*(contagem+1)*(prescale+1)
//1ms = (1/80MHz)*(contagem+1) -> contagem = 79999 (32 bits, sem prescale)
#define CONTAGEM_1MS 79999

//Base de tempo (atualizada somente pela ISR)
volatile uint32_t contador_ms = 0;      //ms acumulados dentro do segundo atual
volatile uint32_t segundos_jogo = 0;    //tempo decorrido da partida (s)
volatile uint8_t  contando = 0;         //1 = partida em andamento

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

	//9. Habilitar o timer (TAEN = 1). Ele conta sempre; o tempo de jogo so e
	//   acumulado quando 'contando' = 1
	TIMER0_CTL_R |= 0x01;
}

// -------------------------------------------------------------------------------
// Funcao Timer0_Read
// Le o valor corrente do timer (GPTMTAR), usado no sorteio do numero secreto
// Parametro de entrada: Nao tem
// Parametro de saida: valor corrente da contagem
uint32_t Timer0_Read(void)
{
	return TIMER0_TAR_R;
}

// -------------------------------------------------------------------------------
// Funcao Tempo_Iniciar
// Zera e inicia a contagem do tempo de jogo
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void Tempo_Iniciar(void)
{
	contando = 0;
	contador_ms = 0;
	segundos_jogo = 0;
	contando = 1;
}

// -------------------------------------------------------------------------------
// Funcao Tempo_Parar
// Para a contagem do tempo de jogo
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void Tempo_Parar(void)
{
	contando = 0;
}

// -------------------------------------------------------------------------------
// Funcao Tempo_Segundos
// Parametro de entrada: Nao tem
// Parametro de saida: tempo decorrido da partida em segundos
uint32_t Tempo_Segundos(void)
{
	return segundos_jogo;
}

// -------------------------------------------------------------------------------
// Funcao Timer0A_Handler
// Rotina de tratamento da interrupcao do Timer0A (a cada 1ms).
// Apenas atualiza a base de tempo: 1000 interrupcoes = 1s
void Timer0A_Handler(void)
{
	//Limpar o flag de interrupcao (ACK)
	TIMER0_ICR_R = 0x01;

	if (contando)
	{
		contador_ms++;
		if (contador_ms >= 1000)
		{
			contador_ms = 0;
			segundos_jogo++;
		}
	}
}

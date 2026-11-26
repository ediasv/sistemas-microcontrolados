// timer.c
// Desenvolvido para a placa EK-TM4C1294XL
// Timer0A periodico de 1ms usado como base de tempo

#include <stdint.h>

#include "tm4c1294ncpdt.h"

//Tempo = (1/sysclk)*(contagem+1)*(prescale+1)
//1ms = (1/80MHz)*(contagem+1) -> contagem = 79999 (32 bits, sem prescale)
#define CONTAGEM_1MS 79999

volatile uint32_t tempo_ms = 0; //ms desde a inicializacao (atualizado somente pela ISR)

//Eventos periodicos. A ISR so incrementa os contadores "ticks_*"; o laco principal
//consome os eventos comparando com seus proprios contadores "tratados_*". Assim a ISR
//e o main nunca escrevem na mesma variavel e nenhum evento se perde.
static volatile uint32_t ticks_100ms = 0; //escrito somente pela ISR
static volatile uint32_t ticks_1s = 0;    //escrito somente pela ISR
static uint32_t tratados_100ms = 0;       //escrito somente pelo laco principal
static uint32_t tratados_1s = 0;          //escrito somente pelo laco principal
static uint8_t cont_ms = 0;               //uso interno da ISR: ms dentro dos 100ms
static uint8_t cont_100ms = 0;            //uso interno da ISR: 100ms dentro de 1s

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
// Curta, sem lacos: so atualiza contadores. O tratamento dos eventos e no main.
void Timer0A_Handler(void)
{
	//Limpar o flag de interrupcao (ACK)
	TIMER0_ICR_R = 0x01;
	tempo_ms++;

	cont_ms++;
	if(cont_ms >= 100) //100ms
	{
		cont_ms = 0;
		ticks_100ms++;
		cont_100ms++;
		if(cont_100ms >= 10) //10 x 100ms = 1s
		{
			cont_100ms = 0;
			ticks_1s++;
		}
	}
}

// -------------------------------------------------------------------------------
// Funcao Timer_Evento100ms
// Informa se passaram 100ms desde a ultima vez que o evento foi consumido
// Parametro de entrada: Nao tem
// Parametro de saida: 1 se ha um evento de 100ms pendente (e o consome), 0 caso contrario
uint8_t Timer_Evento100ms(void)
{
	if(ticks_100ms != tratados_100ms)
	{
		tratados_100ms++;
		return 1;
	}
	return 0;
}

// -------------------------------------------------------------------------------
// Funcao Timer_Evento1s
// Informa se passou 1s desde a ultima vez que o evento foi consumido
// Parametro de entrada: Nao tem
// Parametro de saida: 1 se ha um evento de 1s pendente (e o consome), 0 caso contrario
uint8_t Timer_Evento1s(void)
{
	if(ticks_1s != tratados_1s)
	{
		tratados_1s++;
		return 1;
	}
	return 0;
}

// -------------------------------------------------------------------------------
// Funcao Timer_DescartaEventos
// Descarta os eventos de 100ms e 1s acumulados ate agora (ex.: durante a espera inicial)
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void Timer_DescartaEventos(void)
{
	tratados_100ms = ticks_100ms;
	tratados_1s = ticks_1s;
}

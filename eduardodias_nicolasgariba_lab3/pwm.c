// pwm.c
// Desenvolvido para a placa EK-TM4C1294XL
// PWM por software com o Timer1A (modo periodico) e interrupcao
// Saida em PE0 (sentido horario) ou PE1 (sentido anti-horario), entradas IN1/IN2 do L293
// O enable do L293 (PF2) NAO e alterado aqui

#include <stdint.h>

#include "tm4c1294ncpdt.h"

#define PWM_PERIODO_TICKS 80000 //1ms = 1kHz (80MHz * 1ms)
#define PWM_TICKS_POR_MIL 80    //ticks por 0,1% do periodo (80000 / 1000)
#define PWM_DUTY_MAX      999   //99,9%: o tempo desligado nunca chega a zero

#define PWM_PE0 0x01
#define PWM_PE1 0x02

//Variaveis compartilhadas entre o laco principal e a ISR
static volatile uint8_t  pwm_ativo = 0;     //1 enquanto o Timer1A esta gerando pulsos
static volatile uint8_t  pwm_nivel = 0;     //nivel atual do pino de saida (1 = ligado)
static volatile uint8_t  pwm_pino = PWM_PE0;//mascara do pino de saida (PE0 ou PE1)
static volatile uint32_t pwm_alto_ticks = 0;//tempo ligado, em ticks

// -------------------------------------------------------------------------------
// Funcao PWM_Init
// Configura o Timer1A no modo periodico com interrupcao, sem inicia-lo
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void PWM_Init(void)
{
	//1. Habilitar o timer1 no RCGCTIMER e esperar estar pronto no PRTIMER
	SYSCTL_RCGCTIMER_R |= 0x02;
	while((SYSCTL_PRTIMER_R & 0x02) != 0x02){};

	//2. Desabilitar o timer antes de fazer as alteracoes (TAEN = 0)
	TIMER1_CTL_R &= ~0x01;

	//3. Modo 32 bits, periodico, sem prescale
	TIMER1_CFG_R = 0x00;
	TIMER1_TAMR_R = 0x02;
	TIMER1_TAPR_R = 0x00;
	TIMER1_TAILR_R = PWM_PERIODO_TICKS - 1;

	//4. Limpar o flag de interrupcao (ACK) e habilitar a interrupcao de time-out
	TIMER1_ICR_R = 0x01;
	TIMER1_IMR_R = 0x01;

	//5. Prioridade 2 no NVIC (maior que a do Timer0A, que e 4): o PWM nao espera
	//   a ISR de 1ms. Timer1A e a interrupcao 21, campo dos bits 15-13 do PRI5
	NVIC_PRI5_R = (NVIC_PRI5_R & ~(0x7 << 13)) | (2 << 13);

	//6. Habilitar a interrupcao 21 no NVIC
	NVIC_EN0_R = 1 << 21;
}

// -------------------------------------------------------------------------------
// Funcao PWM_Parar
// Comando de 0%: para o timer e deixa PE0 e PE1 em nivel baixo (sem nenhum pulso)
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void PWM_Parar(void)
{
	pwm_ativo = 0;            //a ISR passa a ignorar qualquer interrupcao pendente
	TIMER1_CTL_R &= ~0x01;    //desabilita o timer
	TIMER1_ICR_R = 0x01;      //limpa o flag de interrupcao
	pwm_nivel = 0;
	GPIO_PORTE_AHB_DATA_BITS_R[PWM_PE0 | PWM_PE1] = 0; //escrita so nos bits PE0 e PE1
}

// -------------------------------------------------------------------------------
// Funcao PWM_Definir
// Define o sentido e o duty cycle do PWM
// Parametro de entrada: sentido - 0 = horario (PE0), 1 = anti-horario (PE1)
//                       duty_permil - duty cycle em decimos de por cento (500 = 50%).
//                       0 para o PWM; acima de 999 e limitado a 999 (99,9%)
// Parametro de saida: Nao tem
void PWM_Definir(uint8_t sentido, uint16_t duty_permil)
{
	if(duty_permil == 0)
	{
		PWM_Parar();
		return;
	}
	if(duty_permil > PWM_DUTY_MAX)
	{
		duty_permil = PWM_DUTY_MAX;
	}

	uint8_t novo_pino = (sentido == 0) ? PWM_PE0 : PWM_PE1;
	uint32_t alto = (uint32_t)duty_permil * PWM_TICKS_POR_MIL;

	if(pwm_ativo && (novo_pino == pwm_pino))
	{
		//PWM ja rodando no mesmo pino: so troca o duty, a ISR usa o novo valor
		pwm_alto_ticks = alto;
		return;
	}

	//Partida (ou troca de pino): para tudo, configura e comeca pelo nivel ligado
	PWM_Parar();
	pwm_pino = novo_pino;
	pwm_alto_ticks = alto;
	pwm_nivel = 1;
	GPIO_PORTE_AHB_DATA_BITS_R[pwm_pino] = pwm_pino;   //liga o pino
	TIMER1_TAILR_R = alto - 1;                         //1o intervalo: tempo ligado
	pwm_ativo = 1;
	TIMER1_CTL_R |= 0x01;                              //habilita o timer
}

// -------------------------------------------------------------------------------
// Funcao Timer1A_Handler
// Rotina de tratamento da interrupcao do Timer1A: a cada estouro troca o nivel do pino
// e carrega no timer o tempo do novo nivel (ligado = duty, desligado = complemento)
void Timer1A_Handler(void)
{
	//Limpar o flag de interrupcao (ACK)
	TIMER1_ICR_R = 0x01;

	if(!pwm_ativo)
	{
		return; //interrupcao pendente de um PWM que acabou de ser parado
	}

	if(pwm_nivel != 0)
	{
		//Acabou o tempo ligado: desliga e carrega o tempo desligado
		TIMER1_TAILR_R = (PWM_PERIODO_TICKS - pwm_alto_ticks) - 1;
		GPIO_PORTE_AHB_DATA_BITS_R[pwm_pino] = 0;
		pwm_nivel = 0;
	}
	else
	{
		//Acabou o tempo desligado: liga e carrega o tempo ligado
		TIMER1_TAILR_R = pwm_alto_ticks - 1;
		GPIO_PORTE_AHB_DATA_BITS_R[pwm_pino] = pwm_pino;
		pwm_nivel = 1;
	}
}

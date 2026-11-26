// gpio.c
// Desenvolvido para a placa EK-TM4C1294XL
// Inicializacao dos ports usados no Lab 3 (motor DC via L293 da PAT DAELN)
// Baseado no exemplo do Prof. Guilherme Peron

#include <stdint.h>

#include "tm4c1294ncpdt.h"

#define GPIO_PORTE  (0x0010) //bit 4
#define GPIO_PORTF  (0x0020) //bit 5

// Mapeamento dos pinos (PAT DAELN, ponte H L293D)
//   PE0  -> IN1 do L293 (sentido: 1 em PE0 e 0 em PE1 = um sentido)
//   PE1  -> IN2 do L293 (sentido: 0 em PE0 e 1 em PE1 = outro sentido)
//   PF2  -> EN1 do L293 (enable da ponte; 0 = motor desabilitado)
//   PE4  -> AIN9, entrada analogica do potenciometro
//   Alimentacao do motor: fonte externa no conector P18 (Vcc no +12V1, GND no GND)
#define MOTOR_PE0   (0x01)
#define MOTOR_PE1   (0x02)
#define MOTOR_PE_MASK (MOTOR_PE0 | MOTOR_PE1)
#define POT_PE4     (0x10)
#define MOTOR_EN_PF2 (0x04)

// -------------------------------------------------------------------------------
// Funcao GPIO_Init
// Inicializa os ports E e F com o acionamento do motor desabilitado
//   PE0, PE1: saidas, iniciam em 0
//   PF2: saida, inicia em 0 (L293 desabilitado)
//   PE4: entrada analogica (AIN9)
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void GPIO_Init(void)
{
	//1a. Ativar o clock para a porta setando o bit correspondente no registrador RCGCGPIO
	SYSCTL_RCGCGPIO_R |= (GPIO_PORTE | GPIO_PORTF);
	//1b.   apos isso verificar no PRGPIO se a porta esta pronta para uso.
	while((SYSCTL_PRGPIO_R & (GPIO_PORTE | GPIO_PORTF)) != (GPIO_PORTE | GPIO_PORTF)){};

	// 2. Limpar o AMSEL para desabilitar a analogica (PE4 e habilitado no passo 8)
	GPIO_PORTE_AHB_AMSEL_R = 0x00;
	GPIO_PORTF_AHB_AMSEL_R = 0x00;

	// 3. Limpar PCTL para selecionar o GPIO
	GPIO_PORTE_AHB_PCTL_R = 0x00;
	GPIO_PORTF_AHB_PCTL_R = 0x00;

	// 4. Motor desabilitado ANTES de configurar como saida: sem glitch no pino
	GPIO_PORTE_AHB_DATA_R &= ~MOTOR_PE_MASK; //PE0 = PE1 = 0
	GPIO_PORTF_AHB_DATA_R &= ~MOTOR_EN_PF2;  //PF2 = 0 (enable do L293 em 0)

	// 5. DIR para 0 se for entrada, 1 se for saida
	GPIO_PORTE_AHB_DIR_R = MOTOR_PE_MASK;    //PE0 e PE1 saidas; PE4 entrada
	GPIO_PORTF_AHB_DIR_R = MOTOR_EN_PF2;     //PF2 saida

	// 6. Limpar os bits AFSEL para 0 para selecionar GPIO sem funcao alternativa
	GPIO_PORTE_AHB_AFSEL_R = 0x00;
	GPIO_PORTF_AHB_AFSEL_R = 0x00;

	// 7. Setar os bits de DEN para habilitar I/O digital
	GPIO_PORTE_AHB_DEN_R = MOTOR_PE_MASK;    //PE0 e PE1 (PE4 e analogico, DEN = 0)
	GPIO_PORTF_AHB_DEN_R = MOTOR_EN_PF2;     //PF2

	// 8. PE4 como entrada analogica AIN9: funcao alternativa + AMSEL (DEN ja esta em 0)
	GPIO_PORTE_AHB_AFSEL_R |= POT_PE4;
	GPIO_PORTE_AHB_AMSEL_R |= POT_PE4;
}

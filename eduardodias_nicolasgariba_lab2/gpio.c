// gpio.c
// Desenvolvido para a placa EK-TM4C1294XL
// Inicializa as portas K, L e M (LCD e teclado matricial da PAT DAELN)
// Baseado no exemplo do Prof. Guilherme Peron

#include <stdint.h>

#include "tm4c1294ncpdt.h"

#define GPIO_PORTK  (0x0200) //bit 9
#define GPIO_PORTL  (0x0400) //bit 10
#define GPIO_PORTM  (0x0800) //bit 11

// -------------------------------------------------------------------------------
// Funcao GPIO_Init
// Inicializa os ports K, L e M
//   PK0-PK7: saidas (barramento de dados D0-D7 do LCD)
//   PM0-PM2: saidas (RS, RW e EN do LCD)
//   PM4-PM7: colunas do teclado, iniciam como entradas (alta impedancia)
//   PL0-PL3: linhas do teclado, entradas com pull-up
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void GPIO_Init(void)
{
	//1a. Ativar o clock para a porta setando o bit correspondente no registrador RCGCGPIO
	SYSCTL_RCGCGPIO_R = (GPIO_PORTK | GPIO_PORTL | GPIO_PORTM);
	//1b.   apos isso verificar no PRGPIO se a porta esta pronta para uso.
	while((SYSCTL_PRGPIO_R & (GPIO_PORTK | GPIO_PORTL | GPIO_PORTM)) != (GPIO_PORTK | GPIO_PORTL | GPIO_PORTM)){};

	// 2. Limpar o AMSEL para desabilitar a analogica
	GPIO_PORTK_AMSEL_R = 0x00;
	GPIO_PORTL_AMSEL_R = 0x00;
	GPIO_PORTM_AMSEL_R = 0x00;

	// 3. Limpar PCTL para selecionar o GPIO
	GPIO_PORTK_PCTL_R = 0x00;
	GPIO_PORTL_PCTL_R = 0x00;
	GPIO_PORTM_PCTL_R = 0x00;

	// 4. DIR para 0 se for entrada, 1 se for saida
	GPIO_PORTK_DIR_R = 0xFF; //PK0-PK7
	GPIO_PORTL_DIR_R = 0x00; //PL0-PL3 entradas
	GPIO_PORTM_DIR_R = 0x07; //PM0-PM2 saidas; PM4-PM7 entradas (alta impedancia)

	// 5. Limpar os bits AFSEL para 0 para selecionar GPIO sem funcao alternativa
	GPIO_PORTK_AFSEL_R = 0x00;
	GPIO_PORTL_AFSEL_R = 0x00;
	GPIO_PORTM_AFSEL_R = 0x00;

	// 6. Setar os bits de DEN para habilitar I/O digital
	GPIO_PORTK_DEN_R = 0xFF; //PK0-PK7
	GPIO_PORTL_DEN_R = 0x0F; //PL0-PL3
	GPIO_PORTM_DEN_R = 0xF7; //PM0-PM2 e PM4-PM7

	// 7. Habilitar resistor de pull-up interno, setar PUR para 1
	GPIO_PORTL_PUR_R = 0x0F; //PL0-PL3 (linhas do teclado)
}

// -------------------------------------------------------------------------------
// Funcao PortK_Output
// Escreve os valores no port K (barramento de dados do LCD)
// Parametro de entrada: Valor a ser escrito
// Parametro de saida: nao tem
void PortK_Output(uint32_t valor)
{
	GPIO_PORTK_DATA_R = valor;
}

// -------------------------------------------------------------------------------
// Funcao PortM_Output
// Escreve os valores nos bits 0 a 2 do port M (RS, RW e EN do LCD)
// sem alterar os bits 4 a 7 (colunas do teclado)
// Parametro de entrada: Valor a ser escrito
// Parametro de saida: nao tem
void PortM_Output(uint32_t valor)
{
	uint32_t temp;
	//vamos zerar somente os bits 0 a 2
	//para uma escrita amigavel
	temp = GPIO_PORTM_DATA_R & 0xF8;
	//agora vamos fazer o OR com o valor recebido na funcao
	temp = temp | valor;
	GPIO_PORTM_DATA_R = temp;
}

// -------------------------------------------------------------------------------
// Funcao PortM_Colunas_DIR
// Configura a direcao dos bits 4 a 7 do port M (colunas do teclado):
// bit em 1 = saida, bit em 0 = entrada (alta impedancia)
// sem alterar os bits 0 a 2 (LCD)
// Parametro de entrada: Valor da direcao dos bits 4 a 7
// Parametro de saida: nao tem
void PortM_Colunas_DIR(uint32_t valor)
{
	uint32_t temp;
	temp = GPIO_PORTM_DIR_R & 0x0F;
	temp = temp | valor;
	GPIO_PORTM_DIR_R = temp;
}

// -------------------------------------------------------------------------------
// Funcao PortM_Colunas_Output
// Escreve os valores nos bits 4 a 7 do port M (colunas do teclado)
// sem alterar os bits 0 a 2 (LCD)
// Parametro de entrada: Valor a ser escrito nos bits 4 a 7
// Parametro de saida: nao tem
void PortM_Colunas_Output(uint32_t valor)
{
	uint32_t temp;
	temp = GPIO_PORTM_DATA_R & 0x0F;
	temp = temp | valor;
	GPIO_PORTM_DATA_R = temp;
}

// -------------------------------------------------------------------------------
// Funcao PortL_Input
// Le os valores de entrada do port L (linhas do teclado)
// Parametro de entrada: Nao tem
// Parametro de saida: o valor da leitura dos bits 0 a 3 do port
uint32_t PortL_Input(void)
{
	return (GPIO_PORTL_DATA_R & 0x0F);
}

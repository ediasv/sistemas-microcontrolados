// adc.c
// Desenvolvido para a placa EK-TM4C1294XL
// ADC0, sequenciador SS3, canal AIN9 (PE4, potenciometro da PAT DAELN)
// Conversao de 12 bits (0 a 4095 para 0V a 3,3V), disparada por software

#include <stdint.h>

#include "tm4c1294ncpdt.h"

#define ADC0_BIT 0x01 //bit 0 do RCGCADC/PRADC
#define SS3_BIT  0x08 //bit 3 do ACTSS/PSSI/RIS/ISC

// -------------------------------------------------------------------------------
// Funcao ADC_Init
// Configura o ADC0 / SS3 para ler o canal AIN9 (PE4) por disparo de software
// O pino PE4 (AFSEL = 1, AMSEL = 1, DEN = 0) ja e configurado no GPIO_Init
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void ADC_Init(void)
{
	//6. Habilitar o clock do ADC0 e esperar ficar pronto no PRADC
	SYSCTL_RCGCADC_R |= ADC0_BIT;
	while((SYSCTL_PRADC_R & ADC0_BIT) != ADC0_BIT){};

	//7. Maxima taxa de amostragem (MCR = 0x7). A fonte de clock do ADC (ADCCC) fica no padrao
	ADC0_PC_R = 0x07;

	//8. Prioridade dos sequenciadores: SS3 com a maior (0) e as demais todas diferentes
	ADC0_SSPRI_R = 0x0123;

	//9. Desabilitar o SS3 para configura-lo
	ADC0_ACTSS_R &= ~SS3_BIT;

	//10. Gatilho por software no SS3 (EM3 = 0x0)
	ADC0_EMUX_R &= ~0xF000;

	//11. Entrada analogica da amostra do SS3: AIN9 (menor que 16, nao precisa do SSEMUX3)
	ADC0_SSMUX3_R = 9;

	//12. Controle da amostra: IE0 (flag no RIS) e END0 (ultima amostra da sequencia)
	ADC0_SSCTL3_R = 0x06;

	//14. Habilitar o SS3
	ADC0_ACTSS_R |= SS3_BIT;
}

// -------------------------------------------------------------------------------
// Funcao ADC_Leitura
// Faz uma conversao e devolve o resultado. A conversao leva poucos microssegundos
// Parametro de entrada: Nao tem
// Parametro de saida: valor lido, de 0 a 4095
uint16_t ADC_Leitura(void)
{
	//Disparar a conversao por software no SS3
	ADC0_PSSI_R = SS3_BIT;

	//Esperar a conversao terminar (polling do RIS)
	while((ADC0_RIS_R & SS3_BIT) == 0){};

	//Ler o resultado (12 bits) da FIFO do SS3
	uint16_t valor = (uint16_t)(ADC0_SSFIFO3_R & 0xFFF);

	//ACK: limpar o flag de conversao concluida
	ADC0_ISC_R = SS3_BIT;

	return valor;
}

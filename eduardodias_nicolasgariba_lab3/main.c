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
void PWM_Init(void);
void Estados_Reinicia(void);
void Estados_Tecla(char c);
void Estados_Evento100ms(void);
void Estados_Evento1s(void);
void ADC_Init(void);
uint16_t ADC_Leitura(void);
void UART_TxChar(char c);
void UART_TxString(const char *s);
void UART_TxUint(uint32_t n);
uint8_t UART_RxChar(char *c);
uint8_t Timer_Evento100ms(void);
uint8_t Timer_Evento1s(void);
void Timer_DescartaEventos(void);

extern volatile uint32_t tempo_ms; //ms desde a inicializacao (timer.c)

//Nada e transmitido antes deste tempo apos o reset; sem isso o terminal recebe lixo
#define ESPERA_INICIAL_MS 2000

int main(void)
{
	PLL_Init();
	SysTick_Init();
	GPIO_Init();
	Timers_Init();
	UART_Init();
	ADC_Init();
	PWM_Init();

	uint8_t iniciado = 0;   //1 depois de enviada a mensagem inicial
	char tecla;

	while (1) //laco principal
	{
		if (!iniciado)
		{
			//Espera inicial sem bloquear: o laco continua girando
			if (tempo_ms >= ESPERA_INICIAL_MS)
			{
				Estados_Reinicia(); //motor parado e mensagem inicial
				Timer_DescartaEventos(); //a contagem comeca a partir daqui
				iniciado = 1;
			}
		}
		else
		{
			if (UART_RxChar(&tecla)) //nao bloqueia: so age se chegou um caractere
			{
				Estados_Tecla(tecla); //a maquina de estados decide o que fazer
			}

			if (Timer_Evento100ms()) //a cada 100ms: atualiza o alvo do potenciometro
			{
				Estados_Evento100ms();
			}

			if (Timer_Evento1s()) //a cada 1s: mostra o alvo (modo potenciometro)
			{
				Estados_Evento1s();
			}
		}
	}
}

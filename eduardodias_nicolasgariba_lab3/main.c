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
void PWM_Definir(uint8_t sentido, uint16_t duty_permil);
uint8_t PWM_Pino(uint8_t sentido);
void PWM_Medir(uint8_t pino);                         //TESTE-PWM
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
	uint8_t sentido = 0;     //TESTE-PWM: 0 = PE0, 1 = PE1
	uint16_t duty = 0;       //TESTE-PWM: em decimos de por cento; 0 = parado

	while (1) //laco principal
	{
		if (!iniciado)
		{
			//Espera inicial sem bloquear: o laco continua girando
			if (tempo_ms >= ESPERA_INICIAL_MS)
			{
				UART_TxString("Motor parado, pressione '*' para iniciar.\r\n");
				Timer_DescartaEventos(); //a contagem comeca a partir daqui
				iniciado = 1;
			}
		}
		else
		{
			if (UART_RxChar(&tecla)) //nao bloqueia: so age se chegou um caractere
			{
				UART_TxChar(tecla); //por enquanto, devolve o caractere ao terminal (eco)

				//TESTE-PWM: comandos provisorios para medir o PWM com analisador logico
				switch (tecla)
				{
					case '5': duty = 500; break;  //50%
					case '8': duty = 800; break;  //80%
					case '0': duty = 999; break;  //99,9%
					case 's': duty = 0;   break;  //0%: sem pulsos
					case 'h': sentido = 0; break; //PE0
					case 'a': sentido = 1; break; //PE1
					case 'm': PWM_Medir(PWM_Pino(sentido)); break; //mede e mostra no terminal
					default: break;
				}
				PWM_Definir(sentido, duty);       //TESTE-PWM
			}

		}
	}
}

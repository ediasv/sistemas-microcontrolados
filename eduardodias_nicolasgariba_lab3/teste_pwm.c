// teste_pwm.c
// TESTE-PWM: codigo provisorio SO PARA VERIFICAR O PWM sem analisador logico.
// Apagar este arquivo (e tirar do projeto) quando o PWM estiver validado.
//
// Le o nivel do pino de saida do PWM por software e cronometra as bordas com o contador
// do SysTick (decrescente, 24 bits, 80MHz = 12,5ns por tick), calculando a media de
// varios periodos. Limites: resolucao de ~0,1us (tempo de uma volta do laco de leitura)
// e usa o mesmo clock do microcontrolador, entao nao detecta erro do proprio clock.

#include <stdint.h>

#include "tm4c1294ncpdt.h"

void UART_TxString(const char *s);
void UART_TxUint(uint32_t n);
void PWM_Parar(void);
uint8_t PWM_Pino(uint8_t sentido);

#define TICKS_POR_US      80
#define PERIODO_ESPERADO  80000   //1ms, o mesmo de pwm.c
#define TIMEOUT_TICKS     240000  //3ms sem borda: considera que nao ha pulsos
#define N_PERIODOS        200

static uint32_t decorrido(uint32_t inicio, uint32_t fim)
{
	return (inicio - fim) & 0x00FFFFFF; //SysTick conta para baixo, 24 bits
}

//Espera o pino chegar ao nivel desejado. Devolve 1 e o instante da borda, ou 0 se estourou o tempo
static uint8_t espera_nivel(uint8_t pino, uint8_t nivel, uint32_t *instante)
{
	uint32_t inicio = NVIC_ST_CURRENT_R;
	while((GPIO_PORTE_AHB_DATA_BITS_R[pino] != 0) != (nivel != 0))
	{
		if(decorrido(inicio, NVIC_ST_CURRENT_R) > TIMEOUT_TICKS)
		{
			return 0;
		}
	}
	*instante = NVIC_ST_CURRENT_R;
	return 1;
}

//Envia um tempo em ticks como microssegundos com uma casa decimal (ex.: 500,0)
static void tx_us(uint32_t ticks)
{
	uint32_t decimos = ticks / (TICKS_POR_US / 10); //ticks / 8 = decimos de us
	UART_TxUint(decimos / 10);
	UART_TxString(".");
	UART_TxUint(decimos % 10);
	UART_TxString(" us");
}

// -------------------------------------------------------------------------------
// Funcao PWM_Medir
// Mede o PWM no pino indicado e mostra o resultado no terminal
// Parametro de entrada: pino - mascara do pino (0x01 = PE0, 0x02 = PE1)
// Parametro de saida: Nao tem
void PWM_Medir(uint8_t pino)
{
	uint32_t t_sobe, t_desce, t_sobe2;
	uint32_t soma_alto = 0, soma_periodo = 0;
	uint32_t validos = 0, descartados = 0;

	UART_TxString("\r\nMedindo...\r\n");

	if(!espera_nivel(pino, 1, &t_sobe))
	{
		UART_TxString("Sem pulsos: nivel fixo em ");
		UART_TxUint(GPIO_PORTE_AHB_DATA_BITS_R[pino] != 0 ? 1 : 0);
		UART_TxString("\r\n");
		return;
	}

	for(uint32_t i = 0; i < N_PERIODOS; i++)
	{
		if(!espera_nivel(pino, 0, &t_desce) || !espera_nivel(pino, 1, &t_sobe2))
		{
			break; //os pulsos pararam durante a medicao
		}
		uint32_t alto = decorrido(t_sobe, t_desce);
		uint32_t periodo = decorrido(t_sobe, t_sobe2);

		//Descarta periodos que uma interrupcao (ex.: a de 1ms) atrapalhou a leitura
		if(periodo > (PERIODO_ESPERADO * 12 / 10) || periodo < (PERIODO_ESPERADO * 8 / 10))
		{
			descartados++;
		}
		else
		{
			soma_alto += alto;
			soma_periodo += periodo;
			validos++;
		}
		t_sobe = t_sobe2;
	}

	if(validos == 0)
	{
		UART_TxString("Nenhum periodo valido medido\r\n");
		return;
	}

	uint32_t alto_medio = soma_alto / validos;
	uint32_t periodo_medio = soma_periodo / validos;

	UART_TxString("Periodo:    "); tx_us(periodo_medio); UART_TxString("\r\n");
	UART_TxString("Ligado:     "); tx_us(alto_medio); UART_TxString("\r\n");
	UART_TxString("Desligado:  "); tx_us(periodo_medio - alto_medio); UART_TxString("\r\n");
	UART_TxString("Duty:       ");
	uint32_t duty_dec = (alto_medio * 1000) / periodo_medio; //em decimos de por cento
	UART_TxUint(duty_dec / 10);
	UART_TxString(".");
	UART_TxUint(duty_dec % 10);
	UART_TxString(" %\r\n");
	UART_TxString("Periodos medidos: "); UART_TxUint(validos);
	UART_TxString(", descartados: "); UART_TxUint(descartados);
	UART_TxString("\r\n");
}

// -------------------------------------------------------------------------------
// Funcao PWM_TesteFixo
// Liga o motor a 100%, SEM PWM: nivel alto fixo no pino do sentido e PF2 = 1.
// Serve para separar problema de PWM de problema de ligacao/alimentacao do motor.
// Parar com PWM_Parar() (tecla 's')
// Parametro de entrada: sentido - 0 = horario, 1 = anti-horario
// Parametro de saida: Nao tem
void PWM_TesteFixo(uint8_t sentido)
{
	PWM_Parar();
	GPIO_PORTE_AHB_DATA_BITS_R[PWM_Pino(sentido)] = PWM_Pino(sentido);
	GPIO_PORTF_AHB_DATA_BITS_R[0x04] = 0x04;
}

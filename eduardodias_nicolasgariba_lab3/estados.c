// estados.c
// Desenvolvido para a placa EK-TM4C1294XL
// Maquina de estados dos menus do terminal (roteiro do Lab 3, passos 2 a 5)
//
//  AGUARDA_INICIO --'*'--> ESCOLHE_MODO --'t'--> TERM_SENTIDO --'h'/'a'--> TERM_VELOCIDADE
//                                |                                              |
//                               'p'                                  '5'..'9'/'0'
//                                v                                              v
//                          POTENCIOMETRO                                  TERM_ATIVO
//  's' em qualquer estado (exceto AGUARDA_INICIO): para o motor e volta a AGUARDA_INICIO
//  Tecla invalida: nao muda de estado, nao aciona o motor e repete a pergunta

#include <stdint.h>

void UART_TxChar(char c);
void UART_TxString(const char *s);
void UART_TxUint(uint32_t n);
void PWM_Definir(uint8_t sentido, uint16_t duty_permil);
uint16_t ADC_Leitura(void);

typedef enum
{
	EST_AGUARDA_INICIO,   //motor parado, espera '*'
	EST_ESCOLHE_MODO,     //espera 'p' ou 't'
	EST_TERM_SENTIDO,     //modo terminal: espera 'h' ou 'a'
	EST_TERM_VELOCIDADE,  //modo terminal: espera '5'..'9' ou '0'
	EST_TERM_ATIVO,       //modo terminal: motor girando; aceita novo sentido/velocidade
	EST_POTENCIOMETRO     //modo potenciometro (a implementar: por enquanto o motor fica parado)
} estado_t;

//Comando-alvo: a mesma representacao para o teclado e para o potenciometro
typedef struct
{
	uint8_t sentido;  //0 = horario, 1 = anti-horario
	uint16_t duty;    //percentual em decimos de por cento (500 = 50%, 999 = 99,9%); 0 = parado
} alvo_t;

static estado_t estado = EST_AGUARDA_INICIO;
static alvo_t alvo = {0, 0};

//Potenciometro (ADC de 12 bits): 0V = 100% em um sentido, centro = 0%, 3,3V = 100% no outro
#define ADC_CENTRO        2048
#define ADC_ZONA_MORTA    40    //+-40 contagens (~2%) em torno do centro valem 0%: o ruido do ADC nao gera alvo
#define DUTY_MAXIMO       999   //99,9%

// Converte a tecla de velocidade em duty (decimos de por cento). Devolve 0 se nao for tecla de velocidade
static uint16_t duty_da_tecla(char c)
{
	if(c >= '5' && c <= '9') { return (uint16_t)((c - '0') * 100); } //'5' = 500 ... '9' = 900
	if(c == '0') { return 999; }                                    //100% (99,9%)
	return 0;
}

// Converte a leitura do ADC em comando-alvo
// Abaixo do centro: sentido horario; acima do centro: sentido anti-horario (escolha nossa: o roteiro nao fixa)
static alvo_t alvo_do_potenciometro(uint16_t adc)
{
	alvo_t a;
	uint32_t distancia = (adc < ADC_CENTRO) ? (uint32_t)(ADC_CENTRO - adc) : (uint32_t)(adc - ADC_CENTRO);

	a.sentido = (adc < ADC_CENTRO) ? 0 : 1;
	if(distancia <= ADC_ZONA_MORTA)
	{
		a.duty = 0;
	}
	else
	{
		uint32_t d = distancia * 1000 / ADC_CENTRO; //0..1000 decimos de por cento
		a.duty = (uint16_t)((d > DUTY_MAXIMO) ? DUTY_MAXIMO : d);
	}
	return a;
}

// Mostra o alvo no terminal, ex.: "Alvo: horario 80.0%"
static void mostra_alvo(void)
{
	UART_TxString("Alvo: ");
	UART_TxString(alvo.sentido == 0 ? "horario " : "anti-horario ");
	UART_TxUint(alvo.duty / 10);
	UART_TxString(".");
	UART_TxUint(alvo.duty % 10);
	UART_TxString("%\r\n");
}

static void mostra_pergunta(void)
{
	switch(estado)
	{
		case EST_AGUARDA_INICIO:
			UART_TxString("Motor parado, pressione '*' para iniciar.\r\n");
			break;
		case EST_ESCOLHE_MODO:
			UART_TxString("Controle pelo potenciometro ('p') ou pelo terminal ('t')?\r\n");
			break;
		case EST_TERM_SENTIDO:
			UART_TxString("Sentido de rotacao: horario ('h') ou anti-horario ('a')?\r\n");
			break;
		case EST_TERM_VELOCIDADE:
			UART_TxString("Velocidade: '5' a '9' = 50% a 90%, '0' = 100%.\r\n");
			break;
		case EST_TERM_ATIVO:
			UART_TxString("Motor ligado. Troque o sentido ('h'/'a') ou a velocidade ('5'-'9','0'). 's' para parar.\r\n");
			break;
		case EST_POTENCIOMETRO:
			UART_TxString("Modo potenciometro: alvo mostrado a cada 1 s (motor ainda parado). 's' para voltar.\r\n");
			break;
	}
}

// -------------------------------------------------------------------------------
// Funcao Estados_Reinicia
// Para o motor, volta ao estado inicial e mostra a mensagem inicial
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void Estados_Reinicia(void)
{
	alvo.duty = 0;
	PWM_Definir(alvo.sentido, 0); //0%: desabilita o L293 e sem pulsos
	estado = EST_AGUARDA_INICIO;
	mostra_pergunta();
}

// -------------------------------------------------------------------------------
// Funcao Estados_Tecla
// Trata uma tecla recebida pela UART conforme o estado atual
// Parametro de entrada: c - caractere recebido
// Parametro de saida: Nao tem
void Estados_Tecla(char c)
{
	UART_TxChar(c); //eco
	UART_TxString("\r\n");

	if(estado != EST_AGUARDA_INICIO && c == 's')
	{
		Estados_Reinicia();
		return;
	}

	switch(estado)
	{
		case EST_AGUARDA_INICIO:
			if(c == '*') { estado = EST_ESCOLHE_MODO; mostra_pergunta(); return; }
			break;

		case EST_ESCOLHE_MODO:
			if(c == 't') { estado = EST_TERM_SENTIDO; mostra_pergunta(); return; }
			if(c == 'p') { estado = EST_POTENCIOMETRO; mostra_pergunta(); return; }
			break;

		case EST_TERM_SENTIDO:
			if(c == 'h' || c == 'a')
			{
				alvo.sentido = (c == 'h') ? 0 : 1;
				mostra_alvo();
				estado = EST_TERM_VELOCIDADE;
				mostra_pergunta();
				return;
			}
			break;

		case EST_TERM_VELOCIDADE:
			if(duty_da_tecla(c) != 0)
			{
				alvo.duty = duty_da_tecla(c);
				mostra_alvo();
				PWM_Definir(alvo.sentido, alvo.duty);
				estado = EST_TERM_ATIVO;
				mostra_pergunta();
				return;
			}
			break;

		case EST_TERM_ATIVO:
			if(c == 'h' || c == 'a')
			{
				alvo.sentido = (c == 'h') ? 0 : 1;
				mostra_alvo();
				PWM_Definir(alvo.sentido, alvo.duty);
				mostra_pergunta();
				return;
			}
			if(duty_da_tecla(c) != 0)
			{
				alvo.duty = duty_da_tecla(c);
				mostra_alvo();
				PWM_Definir(alvo.sentido, alvo.duty);
				mostra_pergunta();
				return;
			}
			break;

		case EST_POTENCIOMETRO:
			break; //so 's' (tratado acima) sai deste estado
	}

	//Tecla invalida neste estado: nada muda, repete a pergunta
	UART_TxString("Tecla invalida.\r\n");
	mostra_pergunta();
}

// -------------------------------------------------------------------------------
// Funcao Estados_Evento100ms
// No modo potenciometro, le o ADC e atualiza o alvo (por enquanto o motor nao e acionado)
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void Estados_Evento100ms(void)
{
	if(estado == EST_POTENCIOMETRO)
	{
		alvo = alvo_do_potenciometro(ADC_Leitura());
	}
}

// -------------------------------------------------------------------------------
// Funcao Estados_Evento1s
// No modo potenciometro, mostra o alvo no terminal uma vez por segundo
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void Estados_Evento1s(void)
{
	if(estado == EST_POTENCIOMETRO)
	{
		mostra_alvo();
	}
}

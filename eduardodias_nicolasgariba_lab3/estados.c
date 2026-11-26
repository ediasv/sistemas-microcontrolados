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
void PWM_Definir(uint8_t sentido, uint16_t duty_permil);

typedef enum
{
	EST_AGUARDA_INICIO,   //motor parado, espera '*'
	EST_ESCOLHE_MODO,     //espera 'p' ou 't'
	EST_TERM_SENTIDO,     //modo terminal: espera 'h' ou 'a'
	EST_TERM_VELOCIDADE,  //modo terminal: espera '5'..'9' ou '0'
	EST_TERM_ATIVO,       //modo terminal: motor girando; aceita novo sentido/velocidade
	EST_POTENCIOMETRO     //modo potenciometro (a implementar: por enquanto o motor fica parado)
} estado_t;

static estado_t estado = EST_AGUARDA_INICIO;
static uint8_t sentido = 0;   //0 = horario, 1 = anti-horario
static uint16_t duty = 0;     //em decimos de por cento (500 = 50%); 0 = parado

// Converte a tecla de velocidade em duty (decimos de por cento). Devolve 0 se nao for tecla de velocidade
static uint16_t duty_da_tecla(char c)
{
	if(c >= '5' && c <= '9') { return (uint16_t)((c - '0') * 100); } //'5' = 500 ... '9' = 900
	if(c == '0') { return 999; }                                    //100% (99,9%)
	return 0;
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
			UART_TxString("Modo potenciometro selecionado (controle ainda nao implementado). 's' para voltar.\r\n");
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
	duty = 0;
	PWM_Definir(sentido, 0); //0%: desabilita o L293 e sem pulsos
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
				sentido = (c == 'h') ? 0 : 1;
				estado = EST_TERM_VELOCIDADE;
				mostra_pergunta();
				return;
			}
			break;

		case EST_TERM_VELOCIDADE:
			if(duty_da_tecla(c) != 0)
			{
				duty = duty_da_tecla(c);
				PWM_Definir(sentido, duty);
				estado = EST_TERM_ATIVO;
				mostra_pergunta();
				return;
			}
			break;

		case EST_TERM_ATIVO:
			if(c == 'h' || c == 'a')
			{
				sentido = (c == 'h') ? 0 : 1;
				PWM_Definir(sentido, duty);
				mostra_pergunta();
				return;
			}
			if(duty_da_tecla(c) != 0)
			{
				duty = duty_da_tecla(c);
				PWM_Definir(sentido, duty);
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

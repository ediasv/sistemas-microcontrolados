// uart.c
// Desenvolvido para a placa EK-TM4C1294XL
// UART0 (PA0 = U0Rx, PA1 = U0Tx, ligada ao terminal via USB/ICDI)
// Configuracao: 9600 bps, 8 bits, paridade impar, 2 stop bits

#include <stdint.h>

#include "tm4c1294ncpdt.h"

#define GPIO_PORTA  (0x0001) //bit 0
#define UART0       (0x0001) //bit 0

//BRD = sysclk/(16*baud) = 80MHz/(16*9600) = 520,8333...
//IBRD = 520; FBRD = int(0,8333*64 + 0,5) = 53
#define UART_IBRD_9600 520
#define UART_FBRD_9600 53

//LCRH: WLEN = 8 bits (0x60), STP2 (0x08), PEN (0x02), EPS = 0 -> impar, FEN (0x10)
#define UART_LCRH_8O2  0x7A

// -------------------------------------------------------------------------------
// Funcao UART_Init
// Configura a UART0 a 9600 bps, paridade impar e 2 stop bits (clock do sistema 80MHz)
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void UART_Init(void)
{
	//1. Habilitar o clock da UART0 e da porta A e esperar ficarem prontas
	SYSCTL_RCGCUART_R |= UART0;
	while((SYSCTL_PRUART_R & UART0) != UART0){};
	SYSCTL_RCGCGPIO_R |= GPIO_PORTA;
	while((SYSCTL_PRGPIO_R & GPIO_PORTA) != GPIO_PORTA){};

	//2. Desabilitar a UART antes de configurar
	UART0_CTL_R &= ~0x01;

	//3. Baud rate (divisor 16, HSE = 0) e clock do sistema como fonte
	UART0_IBRD_R = UART_IBRD_9600;
	UART0_FBRD_R = UART_FBRD_9600;
	UART0_CC_R = 0x00;

	//4. Formato da linha (a escrita no LCRH atualiza o divisor)
	UART0_LCRH_R = UART_LCRH_8O2;

	//5. PA0 e PA1 com funcao alternativa (U0Rx/U0Tx), digitais
	GPIO_PORTA_AHB_AMSEL_R &= ~0x03;
	GPIO_PORTA_AHB_AFSEL_R |= 0x03;
	GPIO_PORTA_AHB_PCTL_R = (GPIO_PORTA_AHB_PCTL_R & ~0xFF) | 0x11;
	GPIO_PORTA_AHB_DEN_R |= 0x03;

	//6. Habilitar UART, transmissor e receptor (UARTEN, TXE, RXE)
	UART0_CTL_R |= 0x301;
}

// -------------------------------------------------------------------------------
// Funcao UART_TxChar
// Envia um caractere, esperando ter espaco na FIFO de transmissao
// Parametro de entrada: c - caractere a enviar
// Parametro de saida: Nao tem
void UART_TxChar(char c)
{
	while((UART0_FR_R & UART_FR_TXFF) != 0){};
	UART0_DR_R = (uint32_t)(uint8_t)c;
}

// -------------------------------------------------------------------------------
// Funcao UART_TxString
// Envia uma string terminada em '\0'
// Parametro de entrada: s - string a enviar
// Parametro de saida: Nao tem
void UART_TxString(const char *s)
{
	while(*s != '\0')
	{
		UART_TxChar(*s++);
	}
}

// -------------------------------------------------------------------------------
// Funcao UART_RxChar
// Le um caractere se houver algum na FIFO de recepcao, sem esperar (nao bloqueia)
// Parametro de entrada: c - ponteiro onde o caractere lido e guardado
// Parametro de saida: 1 se um caractere foi lido, 0 se a FIFO esta vazia
uint8_t UART_RxChar(char *c)
{
	if((UART0_FR_R & UART_FR_RXFE) != 0) //RXFE = 1: FIFO vazia
	{
		return 0;
	}
	*c = (char)(UART0_DR_R & 0xFF); //bits 7:0 = dado; bits 11:8 = flags de erro
	return 1;
}

// -------------------------------------------------------------------------------
// Funcao UART_TxUint
// Envia um numero inteiro sem sinal em decimal (sem usar printf)
// Parametro de entrada: n - numero a enviar
// Parametro de saida: Nao tem
void UART_TxUint(uint32_t n)
{
	char digitos[10]; //uint32_t tem no maximo 10 digitos decimais
	uint8_t i = 0;

	do
	{
		digitos[i++] = (char)('0' + (n % 10)); //digitos saem do menos significativo
		n /= 10;
	} while(n > 0);

	while(i > 0)
	{
		UART_TxChar(digitos[--i]); //envia do mais significativo para o menos
	}
}

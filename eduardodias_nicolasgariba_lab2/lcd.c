// lcd.c
// Desenvolvido para a placa EK-TM4C1294XL
// Rotinas do LCD 16x2 (controlador HD44780) ligado na PAT DAELN:
//   D7-D0 -> PK7-PK0, RS -> PM0, R/W -> PM1, EN -> PM2

#include <stdint.h>

void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);
void PortK_Output(uint32_t valor);
void PortM_Output(uint32_t valor);

#define RS 0x01 //PM0: 0 = instrucao, 1 = dado
#define RW 0x02 //PM1: 0 = escrita, 1 = leitura (sempre 0)
#define EN 0x04 //PM2: enable

// -------------------------------------------------------------------------------
// Funcao LCD_Comando
// Envia um comando (instrucao) para o LCD, RS = 0
//   1. Escrever no barramento de dados o comando;
//   2. Habilitar por 10us o LCD (EN=1) com RS=0;
//   3. Desabilitar o LCD (EN=0) pelo tempo necessario (40us ou 1,64ms)
// Parametro de entrada: codigo do comando
// Parametro de saida: Nao tem
void LCD_Comando(uint8_t comando)
{
	PortK_Output(comando);
	PortM_Output(EN);          //RS=0, RW=0, EN=1
	SysTick_Wait1us(10);
	PortM_Output(0x00);        //EN=0
	if (comando == 0x01 || comando == 0x02)
		SysTick_Wait1ms(2);    //reset e home: no minimo 1,64ms
	else
		SysTick_Wait1us(40);
}

// -------------------------------------------------------------------------------
// Funcao LCD_Dado
// Envia um caractere ASCII para o LCD, RS = 1
//   1. Escrever no barramento de dados o caractere;
//   2. Habilitar por 10us o LCD (EN=1) com RS=1;
//   3. Desabilitar o LCD (EN=0) por 40us
// Parametro de entrada: caractere em ASCII
// Parametro de saida: Nao tem
void LCD_Dado(uint8_t dado)
{
	PortK_Output(dado);
	PortM_Output(RS | EN);     //RS=1, RW=0, EN=1
	SysTick_Wait1us(10);
	PortM_Output(RS);          //EN=0
	SysTick_Wait1us(40);
}

// -------------------------------------------------------------------------------
// Funcao LCD_Init
// Inicializa o LCD conforme a sequencia vista em aula
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void LCD_Init(void)
{
	SysTick_Wait1ms(50);       //espera o LCD estabilizar apos energizar
	LCD_Comando(0x38);         //1. Modo 2 linhas / caractere matriz 5x7
	LCD_Comando(0x06);         //2. Cursor com autoincremento para a direita
	LCD_Comando(0x0E);         //3. Habilitar o display + cursor + nao-pisca
	LCD_Comando(0x01);         //4. Resetar: limpar o display e cursor no home
}

// -------------------------------------------------------------------------------
// Funcao LCD_Limpa
// Limpa o display e leva o cursor para o home
// Parametro de entrada: Nao tem
// Parametro de saida: Nao tem
void LCD_Limpa(void)
{
	LCD_Comando(0x01);
}

// -------------------------------------------------------------------------------
// Funcao LCD_Escreve_String
// Escreve uma string a partir de uma posicao do display (0x80-0x8F na
// primeira linha, 0xC0-0xCF na segunda). Completa o restante da linha com
// espacos, assim nao e necessario limpar o display a cada atualizacao.
// Parametro de entrada: posicao (endereco do display) e string
// Parametro de saida: Nao tem
void LCD_Escreve_String(uint8_t posicao, const char *str)
{
	uint8_t coluna = posicao & 0x0F;

	LCD_Comando(posicao);      //posiciona o cursor
	while (*str != '\0' && coluna < 16)
	{
		LCD_Dado(*str);
		str++;
		coluna++;
	}
	while (coluna < 16)
	{
		LCD_Dado(' ');
		coluna++;
	}
}

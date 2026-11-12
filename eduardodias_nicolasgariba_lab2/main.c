// main.c
// Desenvolvido para a placa EK-TM4C1294XL
// Lab 2 - Jogo de adivinhacao: o jogador digita palpites no teclado matricial
// e recebe dicas (MAIOR/MENOR) no LCD ate acertar o numero secreto (0 a 99).
// Teclas: 0-9 digitam o palpite, * apaga, # confirma, A inicia nova partida.
// Autores: Eduardo Dias e Nicolas Gariba

#include <stdint.h>

void PLL_Init(void);
void SysTick_Init(void);
void GPIO_Init(void);

void LCD_Init(void);
void LCD_Limpa(void);
void LCD_Escreve_String(uint8_t posicao, const char *str);

uint8_t Teclado_Read(void);

void Timers_Init(void);
uint32_t Timer0_Read(void);
void Tempo_Iniciar(void);
void Tempo_Parar(void);
uint32_t Tempo_Segundos(void);

#define LINHA1 0x80 //endereco da primeira posicao da primeira linha
#define LINHA2 0xC0 //endereco da primeira posicao da segunda linha

//Fases do jogo
typedef enum estJogo
{
	AGUARDANDO,     //tela inicial, espera a primeira tecla para sortear
	DIGITANDO,      //compondo o palpite
	MOSTRANDO_DICA, //palpite errado, mostrando MAIOR/MENOR
	ACERTOU         //fim de jogo
} estadosJogo;

estadosJogo estados = AGUARDANDO;

uint8_t numero_secreto = 0;   //0 a 99
uint8_t palpite = 0;          //palpite em digitacao
uint8_t num_digitos = 0;      //digitos ja digitados no palpite (0 a 2)
uint8_t tentativas = 0;

void Converte_Dezena_Unidade(uint32_t valor, char *destino);
void Converte_Centena_Dezena_Unidade(uint32_t valor, char *destino);
void Mostra_Palpite(void);
void Mostra_Tentativas(void);
void Mostra_Acerto(void);
void Nova_Partida(void);
void estado_aguardando_func(uint8_t tecla);
void estado_digitando_func(uint8_t tecla);
void estado_mostrando_dica_func(uint8_t tecla);
void estado_acertou_func(uint8_t tecla);

int main(void)
{
	uint8_t tecla;

	PLL_Init();
	SysTick_Init();
	GPIO_Init();
	LCD_Init();
	Timers_Init();
	Nova_Partida();

	while (1) //laco principal
	{
		tecla = Teclado_Read();
		switch (estados)
		{
			case AGUARDANDO:
				estado_aguardando_func(tecla);
				break;
			case DIGITANDO:
				estado_digitando_func(tecla);
				break;
			case MOSTRANDO_DICA:
				estado_mostrando_dica_func(tecla);
				break;
			case ACERTOU:
				estado_acertou_func(tecla);
				break;
		}
	}
}

// -------------------------------------------------------------------------------
// Funcao Converte_Dezena_Unidade
// Separa um valor de 0 a 99 em dezena e unidade e soma 0x30 a cada digito
// para obter o codigo ASCII correspondente
// Parametro de entrada: valor e endereco onde escrever os dois caracteres
// Parametro de saida: Nao tem
void Converte_Dezena_Unidade(uint32_t valor, char *destino)
{
	if (valor > 99)
		valor = 99;
	destino[0] = (valor / 10) + 0x30; //dezena
	destino[1] = (valor % 10) + 0x30; //unidade
}

// -------------------------------------------------------------------------------
// Funcao Converte_Centena_Dezena_Unidade
// Mesmo que a anterior, para valores de 0 a 999 (tempo de jogo)
// Parametro de entrada: valor e endereco onde escrever os tres caracteres
// Parametro de saida: Nao tem
void Converte_Centena_Dezena_Unidade(uint32_t valor, char *destino)
{
	if (valor > 999)
		valor = 999;
	destino[0] = (valor / 100) + 0x30;       //centena
	destino[1] = ((valor / 10) % 10) + 0x30; //dezena
	destino[2] = (valor % 10) + 0x30;        //unidade
}

// -------------------------------------------------------------------------------
// Funcao Mostra_Palpite
// Linha 1: "Palpite: 42" (mostra somente os digitos ja digitados)
void Mostra_Palpite(void)
{
	char linha[] = "Palpite:   ";

	if (num_digitos == 1)
		linha[9] = palpite + 0x30;
	else if (num_digitos == 2)
		Converte_Dezena_Unidade(palpite, &linha[9]);
	LCD_Escreve_String(LINHA1, linha);
}

// -------------------------------------------------------------------------------
// Funcao Mostra_Tentativas
// Linha 2: "Tentativas: 03"
void Mostra_Tentativas(void)
{
	char linha[] = "Tentativas: 00";

	Converte_Dezena_Unidade(tentativas, &linha[12]);
	LCD_Escreve_String(LINHA2, linha);
}

// -------------------------------------------------------------------------------
// Funcao Mostra_Acerto
// Linha 1: "ACERTOU! N=57" / Linha 2: "Tent:06 T:034s"
void Mostra_Acerto(void)
{
	char linha1[] = "ACERTOU! N=00";
	char linha2[] = "Tent:00 T:000s";

	Converte_Dezena_Unidade(numero_secreto, &linha1[11]);
	Converte_Dezena_Unidade(tentativas, &linha2[5]);
	Converte_Centena_Dezena_Unidade(Tempo_Segundos(), &linha2[10]);
	LCD_Escreve_String(LINHA1, linha1);
	LCD_Escreve_String(LINHA2, linha2);
}

// -------------------------------------------------------------------------------
// Funcao Nova_Partida
// Volta para a tela inicial; o sorteio acontece na proxima tecla pressionada
void Nova_Partida(void)
{
	Tempo_Parar();
	palpite = 0;
	num_digitos = 0;
	tentativas = 0;
	LCD_Limpa();
	LCD_Escreve_String(LINHA1, "Adivinhe: 0-99");
	estados = AGUARDANDO;
}

// -------------------------------------------------------------------------------
// Estado AGUARDANDO
// Na primeira tecla da partida sorteia o numero secreto com o valor corrente
// do GPTMTAR (resto da divisao por 100) e inicia a contagem do tempo.
// Se a tecla for um digito, ela ja e usada como primeiro digito do palpite.
void estado_aguardando_func(uint8_t tecla)
{
	if (tecla != 0)
	{
		numero_secreto = Timer0_Read() % 100;
		tentativas = 0;
		palpite = 0;
		num_digitos = 0;
		Tempo_Iniciar();
		Mostra_Palpite();
		Mostra_Tentativas();
		estados = DIGITANDO;
		estado_digitando_func(tecla);
	}
}

// -------------------------------------------------------------------------------
// Estado DIGITANDO
// 0-9 compoem o palpite (ate dois digitos), * apaga, # confirma
void estado_digitando_func(uint8_t tecla)
{
	if (tecla >= '0' && tecla <= '9')
	{
		if (num_digitos < 2)
		{
			palpite = palpite * 10 + (tecla - 0x30);
			num_digitos++;
			Mostra_Palpite();
		}
	}
	else if (tecla == '*')
	{
		palpite = 0;
		num_digitos = 0;
		Mostra_Palpite();
	}
	else if (tecla == '#' && num_digitos > 0)
	{
		tentativas++;
		if (palpite == numero_secreto)
		{
			Tempo_Parar();
			Mostra_Acerto();
			estados = ACERTOU;
		}
		else
		{
			if (numero_secreto > palpite)
				LCD_Escreve_String(LINHA2, "Dica: MAIOR");
			else
				LCD_Escreve_String(LINHA2, "Dica: MENOR");
			//Apos a dica o palpite e zerado para a proxima tentativa
			//(o LCD continua mostrando o palpite que gerou a dica)
			palpite = 0;
			num_digitos = 0;
			estados = MOSTRANDO_DICA;
		}
	}
	else if (tecla == 'A')
	{
		Nova_Partida();
	}
}

// -------------------------------------------------------------------------------
// Estado MOSTRANDO_DICA
// Um digito ou * volta para a digitacao do proximo palpite
void estado_mostrando_dica_func(uint8_t tecla)
{
	if ((tecla >= '0' && tecla <= '9') || tecla == '*')
	{
		Mostra_Palpite();
		Mostra_Tentativas();
		estados = DIGITANDO;
		estado_digitando_func(tecla);
	}
	else if (tecla == 'A')
	{
		Nova_Partida();
	}
}

// -------------------------------------------------------------------------------
// Estado ACERTOU
// Somente a tecla A inicia uma nova partida
void estado_acertou_func(uint8_t tecla)
{
	if (tecla == 'A')
	{
		Nova_Partida();
	}
}

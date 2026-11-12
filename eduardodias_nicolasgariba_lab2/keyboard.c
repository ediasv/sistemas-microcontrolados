// keyboard.c
// Desenvolvido para a placa EK-TM4C1294XL
// Leitura do teclado matricial 4x4 por varredura (PAT DAELN):
//   PM4-PM7 varrem as colunas, PL0-PL3 leem as linhas (pull-up interno)

#include <stdint.h>

void SysTick_Wait1ms(uint32_t delay);
void PortM_Colunas_DIR(uint32_t valor);
void PortM_Colunas_Output(uint32_t valor);
uint32_t PortL_Input(void);

#define TEMPO_GUARDA 1   //ms ao trocar alta impedancia <-> saida
#define DEBOUNCE     200 //ms de debounce a cada tecla pressionada

//Mapeamento do teclado: teclado_map[PL][PM]
//Na placa o conector do teclado esta invertido (pinos 1..8 = C4..C1, L4..L1):
//PL0..PL3 recebem as colunas 4..1 e PM4..PM7 recebem as linhas 4..1.
//Por isso cada linha desta tabela e uma coluna fisica do teclado, de baixo para cima.
const char teclado_map[4][4] =
{
	/* PM4  PM5  PM6  PM7 */
	{'D', 'C', 'B', 'A'},   //PL0 = coluna 4
	{'#', '9', '6', '3'},   //PL1 = coluna 3
	{'0', '8', '5', '2'},   //PL2 = coluna 2
	{'*', '7', '4', '1'}    //PL3 = coluna 1
};

//Bit de cada coluna no port M
const uint32_t colunas[4] = {0x10, 0x20, 0x40, 0x80}; //PM4, PM5, PM6, PM7

// -------------------------------------------------------------------------------
// Funcao Teclado_Read
// Algoritmo de varredura visto em aula:
//   - Configurar a coluna como saida e as demais como entrada (alta impedancia);
//   - Colocar 0 na coluna;
//   - Verificar o valor de leitura das linhas;
//   - Se algum bit das linhas estiver zerado, a tecla esta nesta coluna e o bit
//     zerado representa a linha: a varredura e encerrada;
//   - Se todos os bits estiverem em 1, varrer a proxima coluna.
// Parametro de entrada: Nao tem
// Parametro de saida: caractere da tecla pressionada ou 0 se nenhuma tecla
uint8_t Teclado_Read(void)
{
	uint32_t linhas;
	uint8_t tecla = 0;
	int c, l;

	for (c = 0; c < 4 && tecla == 0; c++)
	{
		//Configurar a coluna c como saida e as demais como entrada (alta impedancia)
		PortM_Colunas_DIR(colunas[c]);
		//Colocar 0 na coluna
		PortM_Colunas_Output(0x00);
		SysTick_Wait1ms(TEMPO_GUARDA);

		//Verificar o valor de leitura das linhas
		linhas = PortL_Input();
		if (linhas != 0x0F)
		{
			//O bit zerado representa a linha da tecla
			for (l = 0; l < 4; l++)
			{
				if ((linhas & (1 << l)) == 0)
				{
					tecla = teclado_map[l][c];
					break;
				}
			}
			SysTick_Wait1ms(DEBOUNCE);           //debounce ao pressionar
			while (PortL_Input() != 0x0F);       //espera soltar a tecla
			SysTick_Wait1ms(DEBOUNCE);           //debounce ao soltar
		}
	}

	//Todas as colunas de volta para entrada (alta impedancia)
	PortM_Colunas_DIR(0x00);
	SysTick_Wait1ms(TEMPO_GUARDA);
	return tecla;
}

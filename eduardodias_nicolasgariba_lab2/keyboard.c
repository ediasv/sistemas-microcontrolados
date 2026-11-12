#include <stdint.h>
#include "tm4c1294ncpdt.h"

//Prototipos externos que este arquivo precisa usar:
void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);

#define DEBOUNCE_MS 150   //Roteiro: atraso de 100-200ms apos identificar a tecla

//Mapeamento do teclado: teclado_map[PL][PM]
//Na placa o conector do teclado esta invertido (pinos 1..8 = C4..C1, L4..L1):
//PL0..PL3 recebem as colunas 4..1 e PM4..PM7 recebem as linhas 4..1.
//Por isso cada linha desta tabela e uma coluna fisica do teclado, de baixo para cima.
const char teclado_map[4][4] = {
    /* PM4  PM5  PM6  PM7 */
    {'D', 'C', 'B', 'A'},   //PL0 = coluna 4
    {'#', '9', '6', '3'},   //PL1 = coluna 3
    {'0', '8', '5', '2'},   //PL2 = coluna 2
    {'*', '7', '4', '1'}    //PL3 = coluna 1
};

#define COLUNAS_MASK 0xF0 //PM4-PM7 (nunca mexer em PM0-PM2, usados pelo LCD)
#define GUARDA_US    50   //Tempo de guarda apos trocar a coluna ativa

//Volta PM4-PM7 para entrada (alta impedancia), sem alterar os demais bits.
static void Colunas_AltaImpedancia(void) {
    GPIO_PORTM_DIR_R &= ~COLUNAS_MASK;
}

//Varredura do teclado: ativa uma coluna por vez e le as linhas.
//Coluna ativa = saida em nivel baixo; colunas inativas = entrada (alta impedancia).
//Assim, duas teclas apertadas na mesma linha nunca ligam uma saida em 0 a uma saida em 1.
//Retorna o caractere da tecla pressionada ou 0 se nenhuma tecla.
uint8_t Teclado_Read(void) {
    const uint32_t colunas[4] = {0x10, 0x20, 0x40, 0x80};   //PM4, PM5, PM6, PM7

    for (int c = 0; c < 4; c++) {
        //Primeiro DIR, depois DATA: o TM4C ignora escritas em GPIODATA de pinos
        //configurados como entrada, entao o 0 so e registrado com o pino ja saida.
        GPIO_PORTM_DIR_R = (GPIO_PORTM_DIR_R & ~COLUNAS_MASK) | colunas[c];    //a) so ela como saida
        GPIO_PORTM_DATA_R &= ~colunas[c];                                      //b) coluna ativa em 0
        SysTick_Wait1us(GUARDA_US);                                            //c) tempo de guarda

        uint32_t lines = GPIO_PORTL_DATA_R & 0x0F;                             //d) le PL0-PL3

        if (lines != 0x0F) {
            for (int l = 0; l < 4; l++) {
                if (!(lines & (1 << l))) {
                    SysTick_Wait1ms(DEBOUNCE_MS);                //Debounce
                    while((GPIO_PORTL_DATA_R & 0x0F) != 0x0F);   //Trava ate soltar
                    Colunas_AltaImpedancia();
                    return teclado_map[l][c];
                }
            }
        }
    }
    Colunas_AltaImpedancia();
    return 0;
}

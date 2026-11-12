#include <stdint.h>
#include "tm4c1294ncpdt.h"

//Prototipos externos que este arquivo precisa usar:
void SysTick_Wait1ms(uint32_t delay);

#define DEBOUNCE_MS 200   //Slide 25 d): debounce de 200ms a cada tecla pressionada

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
#define GUARDA_MS    1    //Slide 25 c): tempo de guarda de 1ms ao trocar alta impedancia <-> saida

//Todas as colunas (PM4-PM7) como entrada = alta impedancia. Nao altera PM0-PM2.
static void Colunas_AltaImpedancia(void) {
    GPIO_PORTM_DIR_R &= ~COLUNAS_MASK;
    SysTick_Wait1ms(GUARDA_MS);                                            //tempo de guarda
}

//Varredura do teclado (slide 23):
// - configurar a coluna como saida e as demais como entrada (alta impedancia);
// - colocar 0 na coluna;
// - ler as linhas: um bit em 0 indica a linha da tecla e a varredura termina;
// - se todos os bits estiverem em 1, repetir para a proxima coluna.
//Retorna o caractere da tecla pressionada ou 0 se nenhuma tecla.
uint8_t Teclado_Read(void) {
    const uint32_t colunas[4] = {0x10, 0x20, 0x40, 0x80};   //PM4, PM5, PM6, PM7

    for (int c = 0; c < 4; c++) {
        //1) Esta coluna como saida e as demais como entrada (alta impedancia)
        GPIO_PORTM_DIR_R = (GPIO_PORTM_DIR_R & ~COLUNAS_MASK) | colunas[c];
        //2) Colocar 0 na coluna (depois do DIR: o TM4C ignora escritas em GPIODATA
        //   de pinos configurados como entrada)
        GPIO_PORTM_DATA_R &= ~colunas[c];
        SysTick_Wait1ms(GUARDA_MS);                                        //tempo de guarda

        //3) Verificar o valor de leitura das linhas
        uint32_t lines = GPIO_PORTL_DATA_R & 0x0F;

        for (int l = 0; l < 4; l++) {
            if (!(lines & (1 << l))) {
                //4) Bit em 0: tecla na linha l. Trata o bounce (slide 19): espera o
                //   debounce e so aceita a tecla se a linha continuar em 0.
                SysTick_Wait1ms(DEBOUNCE_MS);
                if (GPIO_PORTL_DATA_R & (1 << l)) {
                    break;                                     //era ruido: segue a varredura
                }
                while((GPIO_PORTL_DATA_R & 0x0F) != 0x0F);     //Trava ate soltar
                Colunas_AltaImpedancia();
                return teclado_map[l][c];                      //varredura encerrada
            }
        }
        //5) Todos os bits em 1: proxima coluna
    }
    Colunas_AltaImpedancia();
    return 0;
}

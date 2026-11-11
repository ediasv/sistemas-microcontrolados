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

//Varredura do teclado: ativa uma coluna por vez (nivel baixo) e le as linhas.
//Retorna o caractere da tecla pressionada ou 0 se nenhuma tecla.
uint8_t Teclado_Read(void) {
    uint32_t colunas[4] = {0xEF, 0xDF, 0xBF, 0x7F};

    for (int c = 0; c < 4; c++) {
        GPIO_PORTM_DATA_R = (GPIO_PORTM_DATA_R & 0x0F) | (colunas[c] & 0xF0);
        SysTick_Wait1us(20);

        uint32_t lines = GPIO_PORTL_DATA_R & 0x0F;

        if (lines != 0x0F) {
            for (int l = 0; l < 4; l++) {
                if (!(lines & (1 << l))) {
                    SysTick_Wait1ms(DEBOUNCE_MS);                //Debounce
                    while((GPIO_PORTL_DATA_R & 0x0F) != 0x0F);   //Trava ate soltar
                    return teclado_map[l][c];
                }
            }
        }
    }
    return 0;
}

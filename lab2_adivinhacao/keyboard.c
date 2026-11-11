#include <stdint.h>
#include "tm4c1294ncpdt.h"

//Prototipos externos que este arquivo precisa usar:
void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);

#define DEBOUNCE_MS 150   //Roteiro: atraso de 100-200ms apos identificar a tecla

//Definicao do mapeamento do teclado
const char teclado_map[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
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

#include <stdint.h>
#include "tm4c1294ncpdt.h"

// Definicoes de bits para o registrador RCGCGPIO (Habilitacao de Clock)
#define GPIO_PORTK  (0x0200) // bit 9
#define GPIO_PORTL  (0x0400) // bit 10
#define GPIO_PORTM  (0x0800) // bit 11

void GPIO_Init(void) {
    //Liga Portas K, L e M
    SYSCTL_RCGCGPIO_R |= (GPIO_PORTK | GPIO_PORTL | GPIO_PORTM);
    while((SYSCTL_PRGPIO_R & (GPIO_PORTK | GPIO_PORTL | GPIO_PORTM)) != (GPIO_PORTK | GPIO_PORTL | GPIO_PORTM)){};

    GPIO_PORTK_AMSEL_R = 0x00; GPIO_PORTL_AMSEL_R = 0x00; GPIO_PORTM_AMSEL_R = 0x00;
    GPIO_PORTK_PCTL_R = 0x00;  GPIO_PORTL_PCTL_R = 0x00;  GPIO_PORTM_PCTL_R = 0x00;

    GPIO_PORTK_DIR_R = 0xFF;   //PK0-PK7: Saidas (Dados LCD)
    GPIO_PORTL_DIR_R = 0x00;   //PL0-PL3: Entradas (Linhas do teclado)
    GPIO_PORTM_DIR_R = 0xF7;   //PM0-PM2: Saidas (Controle LCD); PM4-PM7: Saidas (Colunas)

    GPIO_PORTK_AFSEL_R = 0x00; GPIO_PORTL_AFSEL_R = 0x00; GPIO_PORTM_AFSEL_R = 0x00;

    GPIO_PORTK_DEN_R = 0xFF;
    GPIO_PORTL_DEN_R = 0x0F;   //Ativa PL0 ate PL3
    GPIO_PORTM_DEN_R = 0xF7;

    GPIO_PORTL_PUR_R = 0x0F;   //Pull-up interno para as linhas do teclado (PL0-PL3)
}

#include <stdint.h>
#include "tm4c1294ncpdt.h"

//Prototipos externos que este arquivo precisa usar:
void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);

#define RS 0x01 // PM0 (diz se o que enviamos e comando ou dado)
#define RW 0x02 // PM1 (define se e read/write)
#define EN 0x04 // PM2 ("enter" do lcd)

//Os dados dos caracteres ASCII vao pela porta K

//Gera sinal de sincronismo
static void LCD_Pulse_Enable(void) {
    GPIO_PORTM_DATA_R |= EN;
    SysTick_Wait1us(2);
    GPIO_PORTM_DATA_R &= ~EN;
    SysTick_Wait1us(2);
}

//Enviar comando (RS=0)
void LCD_Write_Cmd(uint8_t cmd) {
    GPIO_PORTM_DATA_R &= ~(RS | RW); // RS=0, RW=0
    GPIO_PORTK_DATA_R = cmd;
    LCD_Pulse_Enable();
    SysTick_Wait1ms(2);
}

//Enviar dado (RS=1)
void LCD_Write_Data(uint8_t data) {
    GPIO_PORTM_DATA_R |= RS;   // RS=1
    GPIO_PORTM_DATA_R &= ~RW;  // RW=0
    GPIO_PORTK_DATA_R = data;
    LCD_Pulse_Enable();
    SysTick_Wait1us(50);
}

//Inicia o LCD
void LCD_Init(void) {
    SysTick_Wait1ms(50);
    LCD_Write_Cmd(0x38); // Modo 8 bits, 2 linhas
    LCD_Write_Cmd(0x0C); // Display ligado, cursor desligado
    LCD_Write_Cmd(0x06); // Incremento automatico
    LCD_Write_Cmd(0x01); // Limpa a tela
    SysTick_Wait1ms(5);
}

void LCD_Clear(void) { LCD_Write_Cmd(0x01); }

//Escreve uma string em uma linha (1 ou 2), completando com espacos ate 16
//caracteres. Assim nao e preciso limpar a tela (evita piscar).
void LCD_Print(const char *str, uint8_t linha) {
    uint8_t n = 0;
    if (linha == 1) LCD_Write_Cmd(0x80);
    else LCD_Write_Cmd(0xC0);
    while (*str && n < 16) { LCD_Write_Data(*str++); n++; }
    while (n < 16) { LCD_Write_Data(' '); n++; }
}

#include <stdint.h>
#include "tm4c1294ncpdt.h"

void SysTick_Init(void);
void GPIO_Init(void);
void PLL_Init(void);

void LCD_Init(void);
void LCD_Clear(void);
void LCD_Print(const char *str, uint8_t linha);

uint8_t Teclado_Read(void);

void Timers_Init(void);
uint32_t Timer0_Read(void);
void Tempo_Iniciar(void);
void Tempo_Parar(void);
uint32_t Tempo_Segundos(void);

// Fases do jogo
typedef enum {AGUARDANDO, DIGITANDO, MOSTRANDO_DICA, ACERTOU} Estado_t;

Estado_t estado = AGUARDANDO;

uint8_t numero_secreto = 0;   // 0..99
uint8_t palpite = 0;          // palpite em digitacao
uint8_t num_digitos = 0;      // quantos digitos ja foram digitados (0..2)
uint8_t tentativas = 0;
uint8_t ultima_dica_maior = 0;

void Hardware_Init(void) {
    PLL_Init();
    SysTick_Init();
    GPIO_Init();
    LCD_Init();
    Timers_Init();
}

// Escreve o numero com 'n' digitos (zeros a esquerda) em buf, somando 0x30
static char *Escreve_Num(char *buf, uint32_t valor, uint8_t n) {
    for (int i = n - 1; i >= 0; i--) {
        buf[i] = 0x30 + (valor % 10);
        valor /= 10;
    }
    return buf + n;
}

static char *Escreve_Str(char *buf, const char *s) {
    while (*s) *buf++ = *s++;
    return buf;
}

// Linha 1: "Palpite: 42" (mostra so os digitos ja digitados)
static void Mostra_Palpite(void) {
    char linha[17];
    char *p = Escreve_Str(linha, "Palpite: ");
    if (num_digitos > 0) p = Escreve_Num(p, palpite, num_digitos);
    *p = '\0';
    LCD_Print(linha, 1);
}

// Linha 2: "Tentativas: 03"
static void Mostra_Tentativas(void) {
    char linha[17];
    char *p = Escreve_Str(linha, "Tentativas: ");
    p = Escreve_Num(p, tentativas, 2);
    *p = '\0';
    LCD_Print(linha, 2);
}

static void Mostra_Dica(void) {
    LCD_Print(ultima_dica_maior ? "Dica: MAIOR" : "Dica: MENOR", 2);
}

// "ACERTOU! N=57" / "Tent:06 T:034s"
static void Mostra_Acerto(void) {
    char linha[17];
    char *p;
    uint32_t t = Tempo_Segundos();
    if (t > 999) t = 999;

    p = Escreve_Str(linha, "ACERTOU! N=");
    p = Escreve_Num(p, numero_secreto, 2);
    *p = '\0';
    LCD_Print(linha, 1);

    p = Escreve_Str(linha, "Tent:");
    p = Escreve_Num(p, tentativas, 2);
    p = Escreve_Str(p, " T:");
    p = Escreve_Num(p, t, 3);
    p = Escreve_Str(p, "s");
    *p = '\0';
    LCD_Print(linha, 2);
}

// Volta para a tela inicial; o sorteio acontece na proxima tecla
static void Nova_Partida(void) {
    Tempo_Parar();
    estado = AGUARDANDO;
    palpite = 0;
    num_digitos = 0;
    tentativas = 0;
    LCD_Clear();
    LCD_Print("Adivinhe: 0-99", 1);
}

// Sorteio: valor corrente do GPTMTAR no instante da primeira tecla, mod 100
static void Sorteia_E_Inicia(void) {
    numero_secreto = Timer0_Read() % 100;
    tentativas = 0;
    palpite = 0;
    num_digitos = 0;
    Tempo_Iniciar();
    estado = DIGITANDO;
    Mostra_Palpite();
    Mostra_Tentativas();
}

int main(void) {
    uint8_t tecla;

    Hardware_Init();
    Nova_Partida();

    while (1) {
        tecla = Teclado_Read();
        if (tecla == 0) continue;

        // Tecla A: nova partida a qualquer momento
        if (tecla == 'A') {
            Nova_Partida();
            continue;
        }

        switch (estado) {
            case AGUARDANDO:
                // Primeira tecla da partida: sorteia o numero e inicia o tempo
                Sorteia_E_Inicia();
                break;   // a propria tecla e tratada abaixo (se for digito)

            case MOSTRANDO_DICA:
                // Qualquer tecla volta para a digitacao do proximo palpite
                if ((tecla >= '0' && tecla <= '9') || tecla == '*') {
                    estado = DIGITANDO;
                    palpite = 0;
                    num_digitos = 0;
                    Mostra_Palpite();
                    Mostra_Tentativas();
                }
                break;

            case ACERTOU:
                // So a tecla A (tratada acima) faz algo
                continue;

            default:
                break;
        }

        if (estado != DIGITANDO) continue;

        if (tecla >= '0' && tecla <= '9') {
            // Compoe o palpite, limitado a dois digitos
            if (num_digitos < 2) {
                palpite = palpite * 10 + (tecla - '0');
                num_digitos++;
                Mostra_Palpite();
            }
        }
        else if (tecla == '*') {
            // Apaga o palpite em digitacao
            palpite = 0;
            num_digitos = 0;
            Mostra_Palpite();
        }
        else if (tecla == '#' && num_digitos > 0) {
            // Confirma o palpite
            tentativas++;
            if (palpite == numero_secreto) {
                Tempo_Parar();
                estado = ACERTOU;
                Mostra_Acerto();
            } else {
                ultima_dica_maior = (numero_secreto > palpite);
                estado = MOSTRANDO_DICA;
                Mostra_Palpite();
                Mostra_Dica();
            }
        }
    }
}

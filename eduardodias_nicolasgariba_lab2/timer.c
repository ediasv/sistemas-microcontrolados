#include <stdint.h>
#include "tm4c1294ncpdt.h"

#define TICKS_1MS 80000   //80MHz -> 80000 ticks = 1ms

//Base de tempo (atualizada somente pela ISR)
volatile uint32_t ms_contador = 0;      //ms acumulados dentro do segundo atual
volatile uint32_t segundos_jogo = 0;    //tempo decorrido da partida (s)
volatile uint8_t  contando = 0;         //1 = partida em andamento

//Timer 0A: periodico, interrupcao a cada 1ms
void Timers_Init(void) {
    SYSCTL_RCGCTIMER_R |= 0x01;
    while((SYSCTL_PRTIMER_R & 0x01) != 0x01);

    TIMER0_CTL_R &= ~0x01;          //Desabilita durante a configuracao
    TIMER0_CFG_R = 0x00000000;      //Modo 32 bits
    TIMER0_TAMR_R = 0x02;           //Periodico, contagem decrescente
    TIMER0_TAILR_R = TICKS_1MS - 1; //1ms
    TIMER0_ICR_R = 0x01;
    TIMER0_IMR_R |= 0x01;           //Interrupcao de time-out

    NVIC_EN0_R |= (1 << 19);        //Habilita IRQ 19 (Timer 0A)
    TIMER0_CTL_R |= 0x01;           //Liga o timer (conta sempre)
}

//Le o valor corrente do timer (GPTMTAR) - usado no sorteio
uint32_t Timer0_Read(void) {
    return TIMER0_TAR_R;
}

//Zera e inicia a contagem do tempo de jogo
void Tempo_Iniciar(void) {
    contando = 0;
    ms_contador = 0;
    segundos_jogo = 0;
    contando = 1;
}

void Tempo_Parar(void) {
    contando = 0;
}

uint32_t Tempo_Segundos(void) {
    return segundos_jogo;
}

void Timer0A_Handler(void) {
    TIMER0_ICR_R = 0x01;
    if (contando) {
        ms_contador++;
        if (ms_contador >= 1000) {  //1000 interrupcoes = 1s
            ms_contador = 0;
            segundos_jogo++;
        }
    }
}

# Sistemas Microcontrolados — ELEW31

Repositório centralizador de tudo o que diz respeito à disciplina **Sistemas Microcontrolados (ELEW31)** do curso de Engenharia Eletrônica da UTFPR (Campus Curitiba), ministrada pelo Prof. Guilherme de Santi Peron. Aqui ficam reunidos:

- **Anotações e materiais das aulas** (slides em PDF);
- **Enunciados** dos laboratórios e atividades práticas;
- **Datasheet** do microcontrolador utilizado;
- **Projetos desenvolvidos** (código Assembly e C, fluxogramas e arquivos de projeto Keil).

## Conteúdo da disciplina

A disciplina aborda programação e desenvolvimento de sistemas embarcados, incluindo:

- Arquitetura de microcontroladores;
- Assembly ARM Cortex-M4 e conjunto de instruções Thumb-2;
- Acesso à memória e mapeamento de periféricos;
- Entrada e saída digital (GPIO);
- Interrupções e exceções;
- Máquinas de estados;
- Programação em linguagem C para microcontroladores;
- Temporizadores e contadores;
- Comunicação serial (UART);
- Periféricos e interfaces (teclado matricial, LCD, motores);
- Desenvolvimento de projetos completos com microcontroladores.

## Hardware

Os projetos utilizam as placas **Texas Instruments EK-TM4C1294XL / EK-TM4C129EXL**, baseadas no microcontrolador **TM4C1294NCPDT** (ARM Cortex-M4F). O datasheet completo está em [`Datasheet_TM4C1294NCPDT.pdf`](Datasheet_TM4C1294NCPDT.pdf).

## Estrutura do repositório

| Diretório / arquivo | Conteúdo |
| --- | --- |
| [`aulas/`](aulas) | Slides das aulas em PDF, dos tópicos 07 a 14: GPIO, interrupções, LCD e teclas, máquinas de estados, linguagem C, temporizadores, motores e UART. |
| [`enunciados_labs/`](enunciados_labs) | Enunciados dos laboratórios. Atualmente: **Lab 2 — Jogo de adivinhação** (LCD 16x2, teclado matricial 4x4 e temporizador). |
| [`Datasheet_TM4C1294NCPDT.pdf`](Datasheet_TM4C1294NCPDT.pdf) | Datasheet do microcontrolador TM4C1294NCPDT. |
| [`eduardodias_nicolasgariba_ap0/`](eduardodias_nicolasgariba_ap0) | **AP0**: exercícios em Assembly com ordenação por bolha (`bubble.s`) e verificação de palíndromo, com fluxograma (`Fluxograma.pdf`) e projeto Keil. |
| [`eduardodias_nicolasgariba_lab1/`](eduardodias_nicolasgariba_lab1) | **Laboratório 1**: controle de GPIO em Assembly. O programa aguarda o pressionamento da chave `USR_SW1` e, então, faz o LED1 piscar a cada 0,5 s. Inclui `utils.s`, com configuração do PLL para barramento de 80 MHz e SysTick para atrasos precisos. |

Cada atividade é entregue como pasta de projeto Keil (`.uvprojx`, `.uvoptx`) com o código-fonte, o fluxograma ou diagrama de estados, e um `.zip` com a entrega completa.

## Ferramentas

- **Keil µVision** (IDE e toolchain ARM, usada com os arquivos `.uvprojx`);
- **ARM Cortex-M4** / Assembly Thumb-2;
- **C** (permitida em projetos como o Lab 2, que aceita Assembly ou C);
- **Texas Instruments Tiva C** (`startup.s` vem do pacote de firmware EK-TM4C1294XL);
- **draw.io** (ou similar) para fluxogramas e diagramas de estados.

## Convenções

- Os arquivos `.s` seguem o cabeçalho padrão do curso, com nome do arquivo, placa de destino, professor e data.
- As entregas de atividades são nomeadas com o nome e o último sobrenome dos integrantes da dupla, por exemplo `eduardodias_nicolasgariba_ap0`.
- Arquivos de build gerados pelo Keil (`*.o`, `*.axf`, `*.map`, `*.lst`, etc.) estão no `.gitignore` e não são versionados.

## Autores

- Eduardo Vinicius Dias
- Nicolas Gariba (dupla nas atividades AP0 e Laboratório 1)

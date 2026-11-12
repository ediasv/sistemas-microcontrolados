# Sistemas Microcontrolados — ELEW31

Repositório centralizador de tudo o que diz respeito à disciplina **Sistemas Microcontrolados (ELEW31)** do curso de Engenharia de Computação da UTFPR (Campus Curitiba), ministrada pelo Prof. Guilherme de Santi Peron. Aqui ficam reunidos:

- **Anotações e materiais das aulas** (slides em PDF);
- **Enunciados** dos laboratórios e atividades práticas;
- **Datasheet** do microcontrolador utilizado;
- **Material Extra** fornecido pelo professor, como material de referência de linguagem assembly.
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

## Ferramentas

- **Keil µVision** (IDE e toolchain ARM, usada com os arquivos `.uvprojx`);
- **ARM Cortex-M4** / Assembly Thumb-2;
- **C** (permitida em projetos como o Lab 2, que aceita Assembly ou C);
- **Texas Instruments Tiva C** (`startup.s` vem do pacote de firmware EK-TM4C1294XL);
- **mermaid** para fluxogramas e diagramas de estados.

## Convenções

- Os arquivos `.s` seguem o cabeçalho padrão do curso, com nome do arquivo, placa de destino, professor e data.
- As entregas de atividades são nomeadas com o nome e o último sobrenome dos integrantes da dupla, por exemplo `eduardodias_nicolasgariba_ap0`.
- Arquivos de build gerados pelo Keil (`*.o`, `*.axf`, `*.map`, `*.lst`, etc.) estão no `.gitignore` e não são versionados.

## Autores

- Eduardo Vinicius Dias
- Nicolas Gariba

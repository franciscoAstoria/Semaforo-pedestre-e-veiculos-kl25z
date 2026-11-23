# Semáforos de veículos e pedestres – Zephyr na FRDM-KL25Z

Atividade PSI – Microcontroladores 2 – Aula 07. Feita individualmente, com autorização do professor, com validação cruzada por IA (`docs/validacao_ia.md`).

## O que o sistema faz

| Modo | Veículos (LED RGB da placa) | Pedestres (LEDs externos) |
|---|---|---|
| Dia | verde 3 s → amarelo 1 s → vermelho 4 s | vermelho 4 s → verde 4 s (verde só com carro vermelho) |
| Noturno (chave D8 no GND) | amarelo pisca 1 s / 1 s | vermelho pisca 1 s / 1 s |
| Botão (D2) durante o verde | verde encerra na hora → amarelo 1 s → vermelho | pedestre verde 4 s |

## Arquitetura

```
src/
├── config.h     pinos, tempos, MODO_TESTE
├── hw.c/.h      LEDs, entradas, log "[t ms] ..." e monitor de seguranca
├── sinc.c/.h    mutex, semaforos (bastao) e troca de modo
├── carro.c      3 threads: carro_verde, carro_amarelo, carro_vermelho (+ stub)
├── pedestre.c   2 threads: ped_verde, ped_vermelho (+ stub)
├── entradas.c   1 thread: chave do noturno e botao (com anti-repique)
└── main.c       inicializacao e partida segura
```

- **Uma thread por LED.** Cada thread espera a sua vez (`k_sem_take(&vez_...)`), trava o **mutex** do seu semáforo, acende, espera o tempo, apaga, solta o mutex e passa a vez para o próximo LED.
- **Mutex por semáforo** (`mutex_carro`, `mutex_pedestre`): só um LED de cada semáforo pode estar aceso.
- **Sincronismo entre semáforos:** o carro vermelho acende e dá `libera_pedestre`; o pedestre só abre o verde depois disso. Quando o pedestre volta ao vermelho dá `pedestres_parados`, e só então o carro vermelho apaga. Assim, pedestre verde ⇒ carro vermelho, sempre.
- **Noturno:** entra sempre no fim de um ciclo (ponto seguro). Os dois piscas usam a mesma referência de tempo (`t0_noturno`), então piscam juntos.
- **Botão:** o verde do carro espera "3 s **ou** botão" (`k_sem_take` com timeout). Botão fora do verde é ignorado.
- **Monitor de segurança:** a cada mudança de LED, `hw.c` confere as regras e imprime `ERRO ...` se alguma for violada.

## Ligações

| Função | Pino | Header | Como ligar |
|---|---|---|---|
| Carro vermelho/verde/amarelo | PTB18 / PTB19 | LED RGB da placa | nada a ligar |
| Pedestre vermelho | PTB3 | A3 | A3 → resistor 330 Ω → LED (perna longa) ; perna curta → GND |
| Pedestre verde | PTD5 | D9 | D9 → resistor 330 Ω → LED ; → GND |
| Chave do noturno | PTA13 | D8 | D8 → chave/jumper → GND (fechado = noturno) |
| Botão de travessia | PTD4 | D2 | D2 → botão → GND |

## Compilar, gravar e ver o log

1. Abra a pasta no VS Code com PlatformIO.
2. `PlatformIO: Build` e depois `PlatformIO: Upload`.
3. `PlatformIO: Serial Monitor` (115200). O `log2file` salva o log em um arquivo `.log` na pasta do projeto.

## Testes

Plano completo (modelo V): [`testes/PLANO_DE_TESTES.md`](testes/PLANO_DE_TESTES.md).
Testes unitários: mude `MODO_TESTE` em `src/config.h` para `MODO_TESTE_SO_CARRO` ou `MODO_TESTE_SO_PEDESTRE`.
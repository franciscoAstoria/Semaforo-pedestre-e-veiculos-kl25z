# Plano de testes – Semáforos de veículos e pedestres (modelo V)

**Disciplina:** PSI – Microcontroladores 2 – Aula 07
**Plataforma:** FRDM-KL25Z + Zephyr (PlatformIO)
**Autor:** Francisco Astória – atividade feita individualmente, com autorização do professor, usando uma segunda IA para validação cruzada (ver `docs/validacao_ia.md`).

---

## 1. Objetivo

Verificar que o sistema:

1. controla o semáforo de veículos (3 LEDs, 3 threads) e o de pedestres (2 LEDs, 2 threads) com os tempos pedidos;
2. garante exclusão mútua (mutex) entre os LEDs de cada semáforo;
3. mantém os dois semáforos sincronizados (pedestre verde **somente** com carro vermelho);
4. implementa o modo noturno (piscas de 1 s aceso / 1 s apagado);
5. implementa o botão de travessia, que encerra o verde dos carros de forma segura.

## 2. Modelo V

```
 Requisitos (seção 3) ─────────────────────────────► Testes de SISTEMA (TS)
     \                                                 /
   Arquitetura: 2 semáforos sincronizados ───► Testes de INTEGRAÇÃO (TI)
         \                                         /
       Módulos: carro.c, pedestre.c ───────► Testes UNITÁRIOS (TU)
             \                                 /
                       Implementação (código)
```

| Nível | O que é verificado | Como o módulo é isolado |
|---|---|---|
| Unitário | Cada semáforo sozinho: tempos, ordem, exclusão mútua | `MODO_TESTE` em `src/config.h` troca o outro semáforo por um **stub** (versão simulada, sem LEDs, que só respeita os tempos) |
| Integração | Os dois semáforos juntos: sincronismo e ciclo completo | `MODO_TESTE = MODO_COMPLETO`, sem mexer na chave nem no botão |
| Sistema | Modos de operação: dia, noturno e botão | Sistema completo + chave do noturno + botão |

## 3. Requisitos

| ID | Requisito (do enunciado) |
|---|---|
| R1 | Pedestre: verde aceso por 4 s |
| R2 | Pedestre: vermelho aceso por 4 s |
| R3 | Pedestre: duas threads independentes com mutex entre verde e vermelho |
| R4 | Veículos: verde 3 s, amarelo 1 s, vermelho 4 s |
| R5 | Veículos: três threads independentes com mutex entre os LEDs |
| R6 | Sincronismo entre o semáforo de pedestres e o de veículos |
| R7 | Modo noturno: amarelo dos carros pisca 1 s aceso / 1 s apagado |
| R8 | Modo noturno: vermelho do pedestre pisca 1 s aceso / 1 s apagado |
| R9 | Botão: ativa o verde do pedestre |
| R10 | Botão: bloqueia os carros de forma segura (sempre passando pelo amarelo) |
| R11 | Consistência entre semáforos: nunca há conflito de LEDs |
| R12 | Cada LED controlado por uma thread; threads usam mutex |

## 4. Ambiente e forma de verificação

**Montagem:** semáforo de carros = LED RGB da placa (amarelo = vermelho + verde); pedestre vermelho em A3 e verde em D9 (com resistor de 330 Ω); chave do noturno em D8↔GND; botão em D2↔GND.

**Como verificar:**
- **Tempos:** pelo Monitor Serial (115200). O firmware imprime cada mudança com o tempo em milissegundos, por exemplo `[12003 ms] LED CARRO_VERDE ON` e `[15004 ms] LED CARRO_VERDE OFF`. A diferença entre as duas linhas é a duração (3001 ms ≈ 3 s). Tolerância aceita: **±0,1 s**.
- **Segurança:** a cada mudança de LED, o próprio firmware confere as regras (só um LED aceso por semáforo; pedestre verde só com carro vermelho). Se alguma for quebrada, aparece uma linha `ERRO ...` no monitor. Em todos os testes, o esperado é **nenhuma linha `ERRO`**.
- **Visual:** observação dos LEDs; nos testes de sistema, vídeo curto dos LEDs.

**Registro:** em cada execução, o tempo medido e o resultado de cada caso são anotados junto ao respectivo caso de teste, com o trecho correspondente do Monitor Serial como evidência. Nos testes de sistema, a evidência é complementada por vídeo dos LEDs.

---

## 5. Testes unitários (TU)

### 5.1 Semáforo de veículos
**Configuração:** `MODO_TESTE = MODO_TESTE_SO_CARRO` (o pedestre vira stub). Gravar o firmware na placa, apertar RESET e observar pelo menos 3 ciclos.

| ID | Req. | Procedimento | Esperado |
|---|---|---|---|
| TU-V1 | R4 | Medir no monitor o tempo de `CARRO_VERDE` ON→OFF | 3,0 s |
| TU-V2 | R4 | Medir `CARRO_AMARELO` ON→OFF | 1,0 s |
| TU-V3 | R4 | Medir `CARRO_VERMELHO` ON→OFF | 4,0 s |
| TU-V4 | R4, R5 | Observar a sequência das linhas ON | sempre verde → amarelo → vermelho → verde |
| TU-V5 | R5, R12 | Conferir que um LED só acende depois do OFF do anterior; procurar `ERRO` | nunca 2 LEDs acesos; nenhum `ERRO` |

### 5.2 Semáforo de pedestres
**Configuração:** `MODO_TESTE = MODO_TESTE_SO_PEDESTRE` (o carro vira stub). Observar pelo menos 3 ciclos.

| ID | Req. | Procedimento | Esperado |
|---|---|---|---|
| TU-P1 | R1 | Medir `PED_VERDE` ON→OFF | 4,0 s |
| TU-P2 | R2 | Medir `PED_VERMELHO` ON→OFF | 4,0 s |
| TU-P3 | R3, R12 | Conferir que verde e vermelho nunca ficam acesos juntos; procurar `ERRO` | nunca os 2 acesos; nenhum `ERRO` |

---

## 6. Testes de integração (TI)

**Configuração:** `MODO_TESTE = MODO_COMPLETO`, chave do noturno desligada, sem apertar o botão. Observar pelo menos 3 ciclos.

| ID | Req. | Procedimento | Esperado |
|---|---|---|---|
| TI-1 | R6, R11 | Conferir no monitor a ordem: `CARRO_VERMELHO ON` antes de `PED_VERDE ON`, e `PED_VERDE OFF` antes de `CARRO_VERMELHO OFF` | pedestre verde sempre "dentro" do carro vermelho |
| TI-2 | R6, R11 | Enquanto o carro está verde ou amarelo, conferir o pedestre | pedestre vermelho aceso o tempo todo |
| TI-3 | R2, R4, R6 | Medir o tempo entre dois `CARRO_VERDE ON` seguidos | 8,0 s (3 + 1 + 4) |
| TI-4 | R6 | Apertar RESET e olhar as primeiras linhas | `PED_VERMELHO ON` aparece antes do primeiro `CARRO_VERDE ON` (partida segura) |
| TI-5 | R11 | Procurar linhas `ERRO` em todo o teste | nenhuma |

### 6.1 TI-6 – Teste do teste (injeção de falha)

Serve para mostrar que os testes conseguem detectar um erro.

1. Em `src/carro.c`, troque temporariamente `k_msleep(T_CARRO_AMARELO_MS);` por `k_msleep(1500);`.
2. Grave o firmware na placa e repita TU-V2 e TI-3.
3. **Esperado:** amarelo com 1,5 s (TU-V2 reprova) e ciclo com 8,5 s (TI-3 reprova).
4. Desfaça a alteração. O código com defeito não entra no repositório; só o resultado do teste é registrado.

| ID | Procedimento | Esperado |
|---|---|---|
| TI-6 | Amarelo alterado para 1,5 s | TU-V2 e TI-3 reprovam |

---

## 7. Testes de sistema (TS)

**Configuração:** `MODO_TESTE = MODO_COMPLETO`.

### 7.1 Modo noturno
**Procedimento:** ligar a placa com a chave desligada; depois de ~10 s, ligar a chave (D8 no GND); deixar piscar por ~15 s; desligar a chave; observar mais 2 ciclos.

| ID | Req. | Procedimento | Esperado |
|---|---|---|---|
| TS-N1 | R7 | Medir o amarelo no noturno | 1 s aceso / 1 s apagado |
| TS-N2 | R8 | Medir o vermelho do pedestre no noturno | 1 s aceso / 1 s apagado, junto com o amarelo |
| TS-N3 | R11 | Observar os outros LEDs durante o noturno | carro verde, carro vermelho e pedestre verde apagados |
| TS-N4 | R7, R8 | Medir o tempo entre `EVT CHAVE NOTURNO LIGADA` e `EVT MODO NOTURNO` | entra no fim do ciclo (até ~8 s), nunca no meio de uma travessia |
| TS-N5 | R6, R11 | Desligar a chave e observar a volta | `EVT MODO DIA` em até ~2 s; carro verde em até ~3 s, sempre depois do pedestre vermelho |

### 7.2 Botão de travessia
**Procedimento:** com o sistema no modo dia, apertar o botão (a) no meio de um verde de carro, (b) durante o amarelo, (c) durante o vermelho, (d) várias vezes rápido durante um verde e (e) com o noturno ligado.

| ID | Req. | Procedimento | Esperado |
|---|---|---|---|
| TS-B1 | R9, R10 | (a) Botão no meio do verde | `EVT TRAVESSIA SOLICITADA`; verde apaga na hora; amarelo 1 s; carro vermelho; pedestre verde 4 s |
| TS-B2 | R10 | (b), (c) e (e): botão fora do verde | `EVT TRAVESSIA IGNORADA`; o ciclo não muda |
| TS-B3 | R10 | (d) Vários apertos rápidos no mesmo verde | no máximo uma `TRAVESSIA SOLICITADA` por verde; o sistema não trava |
| TS-B4 | R11 | Procurar linhas `ERRO` nos testes de sistema | nenhuma |

---

## 8. Matriz de rastreabilidade

| Req. | Unitário | Integração | Sistema |
|---|---|---|---|
| R1 | TU-P1 | TI-3 | — |
| R2 | TU-P2 | TI-3 | — |
| R3 | TU-P3 | TI-5 | — |
| R4 | TU-V1, TU-V2, TU-V3, TU-V4 | TI-3, TI-6 | — |
| R5 | TU-V4, TU-V5 | TI-5 | — |
| R6 | — | TI-1, TI-2, TI-3, TI-4 | TS-N5 |
| R7 | — | — | TS-N1, TS-N4 |
| R8 | — | — | TS-N2, TS-N4 |
| R9 | — | — | TS-B1 |
| R10 | — | — | TS-B1, TS-B2, TS-B3 |
| R11 | TU-V5, TU-P3 | TI-1, TI-2, TI-5 | TS-N3, TS-N5, TS-B4 |
| R12 | TU-V5, TU-P3 | TI-5 | — |

## 9. Ordem de execução

Os testes acompanham as fases de desenvolvimento do código:

| Fase do código | Testes executados |
|---|---|
| Fase 2 – semáforo de pedestres | TU-P1 a TU-P3 |
| Fase 3 – semáforo de veículos | TU-V1 a TU-V5 |
| Fase 4 – integração | TI-1 a TI-6 |
| Fase 5 – modo noturno | TS-N1 a TS-N5 |
| Fase 6 – botão de travessia | TS-B1 a TS-B4 |
| Código final | Regressão: TU (trocando `MODO_TESTE`) e TI |

## 10. Critério de aceite

O sistema é aceito quando todos os casos de TU, TI e TS estiverem OK, nenhum teste tiver mostrado linha `ERRO` e o TI-6 tiver reprovado como esperado.

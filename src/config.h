/*
 * config.h - pinos, tempos e modo de teste do sistema de semaforos
 * Placa: FRDM-KL25Z  |  RTOS: Zephyr (PlatformIO)
 */
#ifndef SEMAFORO_CONFIG_H
#define SEMAFORO_CONFIG_H

/* ===================== PINOS ===================== */

/* Semaforo de CARROS: LED RGB da propria placa (acende com nivel 0).
 * Amarelo = vermelho + verde acesos juntos. */
#define PINO_RGB_VERMELHO   18  /* PTB18 */
#define PINO_RGB_VERDE      19  /* PTB19 */

/* Semaforo de PEDESTRES: LEDs externos com resistor de 220-330 ohm
 * (anodo no pino, catodo no GND -> acendem com nivel 1). */
#define PINO_PED_VERMELHO   3   /* PTB3 - header A3 */
#define PINO_PED_VERDE      5   /* PTD5 - header D9 */

/* Entradas com pull-up interno: ativas quando ligadas ao GND. */
#define PINO_CHAVE_NOTURNO  13  /* PTA13 - header D8 (fio/chave para GND = noturno) */
#define PINO_BOTAO          4   /* PTD4  - header D2 (botao para GND) */

/* ===================== TEMPOS (ms) ===================== */
#define T_CARRO_VERDE_MS      3000
#define T_CARRO_AMARELO_MS    1000
#define T_CARRO_VERMELHO_MS   4000  /* = tempo do pedestre verde        */
#define T_PED_VERDE_MS        4000
#define T_PED_VERMELHO_MS     4000  /* = carro verde + carro amarelo    */
#define T_PISCA_MS            1000  /* noturno: 1 s aceso / 1 s apagado */
#define T_LEITURA_ENTRADAS_MS 20    /* periodo de leitura chave/botao   */

/* ===================== MODO DE TESTE =====================
 * MODO_COMPLETO          -> sistema inteiro (uso normal)
 * MODO_TESTE_SO_CARRO    -> teste unitario do semaforo de carros
 *                           (pedestre substituido por um "stub" sem LEDs)
 * MODO_TESTE_SO_PEDESTRE -> teste unitario do semaforo de pedestres
 *                           (carro substituido por um "stub" sem LEDs)
 */
#define MODO_COMPLETO          0
#define MODO_TESTE_SO_CARRO    1
#define MODO_TESTE_SO_PEDESTRE 2

#ifndef MODO_TESTE
#define MODO_TESTE MODO_COMPLETO
#endif

/* ===================== THREADS ===================== */
#define PILHA_THREAD   1024
#define PRIO_LEDS      5     /* threads dos LEDs                         */
#define PRIO_ENTRADAS  4     /* leitura de chave/botao (um pouco maior)  */

#endif /* SEMAFORO_CONFIG_H */

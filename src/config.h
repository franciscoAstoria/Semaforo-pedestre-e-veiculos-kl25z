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

/* ===================== TEMPOS (ms) ===================== */
#define T_CARRO_VERDE_MS      3000
#define T_CARRO_AMARELO_MS    1000
#define T_CARRO_VERMELHO_MS   4000  /* = tempo do pedestre verde        */
#define T_PED_VERDE_MS        4000
#define T_PED_VERMELHO_MS     4000  /* = carro verde + carro amarelo    */

#endif /* SEMAFORO_CONFIG_H */

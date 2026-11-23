/*
 * hw.h - camada de hardware: LEDs, entradas e log com carimbo de tempo
 */
#ifndef SEMAFORO_HW_H
#define SEMAFORO_HW_H

#include <stdbool.h>

typedef enum {
    CARRO_VERDE = 0,
    CARRO_AMARELO,
    CARRO_VERMELHO,
    PED_VERDE,
    PED_VERMELHO,
    NUM_LEDS
} led_t;

int  hw_init(void);
bool hw_pronto(void);

/* Acende/apaga um LED, imprime "[t ms] LED NOME ON/OFF" e confere
 * as regras de seguranca (imprime "[t ms] ERRO ..." se alguma falhar). */
void led_set(led_t led, bool aceso);

/* Imprime "[t ms] EVT <mensagem>" */
void log_evento(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

bool hw_chave_noturno(void); /* true = chave ligada (modo noturno pedido) */
bool hw_botao(void);         /* true = botao apertado */

#endif /* SEMAFORO_HW_H */

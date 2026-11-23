/*
 * main.c - inicializacao
 * Fase 1: teste de hardware (cada LED acende 1 s, em sequencia).
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "config.h"
#include "hw.h"

int main(void)
{
    if (hw_init() != 0) {
        return 0;
    }

    static const led_t ordem[] = {
        CARRO_VERDE, CARRO_AMARELO, CARRO_VERMELHO, PED_VERDE, PED_VERMELHO,
    };

    printk("# Fase 1 - teste de hardware: cada LED acende por 1 s\n");
    while (1) {
        for (size_t i = 0; i < ARRAY_SIZE(ordem); i++) {
            led_set(ordem[i], true);
            k_msleep(1000);
            led_set(ordem[i], false);
        }
    }

    return 0;
}

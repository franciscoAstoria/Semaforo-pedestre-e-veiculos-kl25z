/*
 * carro.c - semaforo de VEICULOS: 3 threads, uma por LED
 *           verde 3 s -> amarelo 1 s -> vermelho 4 s (enquanto o pedestre atravessa)
 */
#include <zephyr/kernel.h>

#include "config.h"
#include "hw.h"
#include "sinc.h"

/* ===================== STUB (teste unitario do pedestre) =====================
 * Simula o semaforo de carros SEM acender LED: so respeita o protocolo e os
 * tempos, para testar o semaforo de pedestres isolado. */

static void carro_stub_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        k_sem_take(&vez_carro_verde, K_FOREVER);
        log_evento("STUB carro verde+amarelo (%d ms)", T_PED_VERMELHO_MS);
        k_msleep(T_PED_VERMELHO_MS);
        log_evento("STUB carro vermelho");
        k_sem_give(&libera_pedestre);
        k_sem_take(&pedestres_parados, K_FOREVER);
        k_sem_give(&vez_carro_verde);
    }
}

K_THREAD_DEFINE(carro_stub, PILHA_THREAD, carro_stub_thread,
                NULL, NULL, NULL, PRIO_LEDS, 0, 0);

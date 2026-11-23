/*
 * pedestre.c - semaforo de PEDESTRES: 2 threads, uma por LED
 *              vermelho enquanto os carros andam (4 s) -> verde 4 s
 */
#include <zephyr/kernel.h>

#include "config.h"
#include "hw.h"
#include "sinc.h"

/* ===================== IMPLEMENTACAO REAL ===================== */

static void ped_vermelho_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        k_sem_take(&vez_ped_vermelho, K_FOREVER);
        k_mutex_lock(&mutex_pedestre, K_FOREVER);

        led_set(PED_VERMELHO, true);
        k_sem_give(&pedestres_parados);         /* avisa: pedestre parado   */

        /* fica vermelho ate o semaforo de carros liberar */
        k_sem_take(&libera_pedestre, K_FOREVER);
        led_set(PED_VERMELHO, false);

        k_mutex_unlock(&mutex_pedestre);
        k_sem_give(&vez_ped_verde);
    }
}

static void ped_verde_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        k_sem_take(&vez_ped_verde, K_FOREVER);
        k_mutex_lock(&mutex_pedestre, K_FOREVER);

        led_set(PED_VERDE, true);
        k_msleep(T_PED_VERDE_MS);
        led_set(PED_VERDE, false);

        k_mutex_unlock(&mutex_pedestre);
        k_sem_give(&vez_ped_vermelho);
    }
}

K_THREAD_DEFINE(ped_vermelho, PILHA_THREAD, ped_vermelho_thread,
                NULL, NULL, NULL, PRIO_LEDS, 0, 0);
K_THREAD_DEFINE(ped_verde, PILHA_THREAD, ped_verde_thread,
                NULL, NULL, NULL, PRIO_LEDS, 0, 0);

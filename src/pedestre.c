/*
 * pedestre.c - semaforo de PEDESTRES: 2 threads, uma por LED
 *              vermelho enquanto os carros andam (4 s) -> verde 4 s
 */
#include <zephyr/kernel.h>

#include "config.h"
#include "hw.h"
#include "sinc.h"

#if MODO_TESTE != MODO_TESTE_SO_CARRO
/* ===================== IMPLEMENTACAO REAL ===================== */

/* Modo noturno: confere o modo no INICIO de cada periodo de 2 s
 * (o carro decide sair no meio do periodo). */
static void vermelho_pisca_noturno(void)
{
    int64_t t = t0_noturno;

    while (1) {
        dormir_ate(t);
        if (modo != MODO_NOTURNO) {
            return;
        }
        led_set(PED_VERMELHO, true);
        dormir_ate(t + T_PISCA_MS);
        led_set(PED_VERMELHO, false);
        t += 2 * T_PISCA_MS;
    }
}

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
        while (modo == MODO_NOTURNO) {          /* liberado para piscar     */
            led_set(PED_VERMELHO, false);
            vermelho_pisca_noturno();
            led_set(PED_VERMELHO, true);        /* volta ao vermelho fixo   */
            k_sem_give(&pedestres_parados);
            k_sem_take(&libera_pedestre, K_FOREVER);
        }
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

#else /* MODO_TESTE == MODO_TESTE_SO_CARRO */
/* ===================== STUB (teste unitario do carro) =====================
 * Simula o semaforo de pedestres SEM acender LED. */

static void pedestre_stub_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    k_sem_take(&vez_ped_vermelho, K_FOREVER);   /* partida dada pelo main */
    while (1) {
        k_sem_give(&pedestres_parados);
        k_sem_take(&libera_pedestre, K_FOREVER);
        if (modo == MODO_NOTURNO) {
            while (modo == MODO_NOTURNO) {
                k_msleep(10);
            }
            continue;
        }
        log_evento("STUB pedestre atravessando (%d ms)", T_CARRO_VERMELHO_MS);
        k_msleep(T_CARRO_VERMELHO_MS);
    }
}

K_THREAD_DEFINE(pedestre_stub, PILHA_THREAD, pedestre_stub_thread,
                NULL, NULL, NULL, PRIO_LEDS, 0, 0);
#endif

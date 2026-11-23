/*
 * carro.c - semaforo de VEICULOS: 3 threads, uma por LED
 *           verde 3 s -> amarelo 1 s -> vermelho 4 s (enquanto o pedestre atravessa)
 */
#include <zephyr/kernel.h>

#include "config.h"
#include "hw.h"
#include "sinc.h"

#if MODO_TESTE != MODO_TESTE_SO_PEDESTRE
/* ===================== IMPLEMENTACAO REAL ===================== */

static void carro_verde_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        k_sem_take(&vez_carro_verde, K_FOREVER);
        k_mutex_lock(&mutex_carro, K_FOREVER);

        led_set(CARRO_VERDE, true);
        k_msleep(T_CARRO_VERDE_MS);
        led_set(CARRO_VERDE, false);

        k_mutex_unlock(&mutex_carro);
        k_sem_give(&vez_carro_amarelo);  /* passa sempre pelo amarelo */
    }
}

/* Modo noturno: 1 s aceso / 1 s apagado. A decisao de sair e tomada no
 * MEIO do periodo (LED apagado); o pedestre confere no INICIO do periodo,
 * entao os dois sempre saem juntos, sem disputa. */
static void amarelo_pisca_noturno(void)
{
    int64_t t = t0_noturno;

    while (1) {
        dormir_ate(t);
        led_set(CARRO_AMARELO, true);
        dormir_ate(t + T_PISCA_MS);
        led_set(CARRO_AMARELO, false);

        if (!pedido_noturno) {
            modo = MODO_DIA;
            log_evento("MODO DIA");
            dormir_ate(t + 2 * T_PISCA_MS);
            return;
        }
        t += 2 * T_PISCA_MS;
    }
}

static void carro_amarelo_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        k_sem_take(&vez_carro_amarelo, K_FOREVER);
        k_mutex_lock(&mutex_carro, K_FOREVER);

        if (modo == MODO_NOTURNO) {
            amarelo_pisca_noturno();
            k_mutex_unlock(&mutex_carro);
            /* so volta ao verde depois do pedestre estar vermelho fixo */
            k_sem_take(&pedestres_parados, K_FOREVER);
            k_sem_give(&vez_carro_verde);
            continue;
        }

        led_set(CARRO_AMARELO, true);
        k_msleep(T_CARRO_AMARELO_MS);
        led_set(CARRO_AMARELO, false);

        k_mutex_unlock(&mutex_carro);
        k_sem_give(&vez_carro_vermelho);
    }
}

static void carro_vermelho_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        k_sem_take(&vez_carro_vermelho, K_FOREVER);
        k_mutex_lock(&mutex_carro, K_FOREVER);

        led_set(CARRO_VERMELHO, true);
        k_sem_give(&libera_pedestre);              /* carros parados       */
        k_sem_take(&pedestres_parados, K_FOREVER); /* travessia terminou   */
        led_set(CARRO_VERMELHO, false);

        k_mutex_unlock(&mutex_carro);

        if (pedido_noturno) {        /* fim de ciclo = ponto seguro */
            entra_modo_noturno();
            k_sem_give(&vez_carro_amarelo);
            continue;
        }
        k_sem_give(&vez_carro_verde);
    }
}

K_THREAD_DEFINE(carro_verde, PILHA_THREAD, carro_verde_thread,
                NULL, NULL, NULL, PRIO_LEDS, 0, 0);
K_THREAD_DEFINE(carro_amarelo, PILHA_THREAD, carro_amarelo_thread,
                NULL, NULL, NULL, PRIO_LEDS, 0, 0);
K_THREAD_DEFINE(carro_vermelho, PILHA_THREAD, carro_vermelho_thread,
                NULL, NULL, NULL, PRIO_LEDS, 0, 0);

#else /* MODO_TESTE == MODO_TESTE_SO_PEDESTRE */
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

        if (pedido_noturno) {
            entra_modo_noturno();
            int64_t t = t0_noturno;
            while (1) {
                dormir_ate(t + T_PISCA_MS);   /* meio do periodo */
                if (!pedido_noturno) {
                    break;
                }
                t += 2 * T_PISCA_MS;
            }
            modo = MODO_DIA;
            log_evento("MODO DIA");
            k_sem_take(&pedestres_parados, K_FOREVER);
        }
        k_sem_give(&vez_carro_verde);
    }
}

K_THREAD_DEFINE(carro_stub, PILHA_THREAD, carro_stub_thread,
                NULL, NULL, NULL, PRIO_LEDS, 0, 0);
#endif

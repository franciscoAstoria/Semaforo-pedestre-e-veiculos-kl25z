/*
 * entradas.c - thread que le a chave do modo noturno e o botao de travessia
 */
#include <zephyr/kernel.h>

#include "config.h"
#include "hw.h"
#include "sinc.h"

typedef struct {
    bool estavel;   /* valor ja filtrado   */
    bool anterior;  /* ultima leitura crua */
} filtro_t;

/* Anti-repique simples: aceita o novo valor quando duas leituras seguidas
 * (T_LEITURA_ENTRADAS_MS de intervalo) concordam. Retorna true se mudou. */
static bool filtra(filtro_t *f, bool leitura)
{
    bool mudou = false;

    if (leitura == f->anterior && leitura != f->estavel) {
        f->estavel = leitura;
        mudou = true;
    }
    f->anterior = leitura;
    return mudou;
}

static void entradas_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    filtro_t chave = {0};

    while (!hw_pronto()) {
        k_msleep(10);
    }

    while (1) {
        if (filtra(&chave, hw_chave_noturno())) {
            pedido_noturno = chave.estavel;
            log_evento("CHAVE NOTURNO %s", chave.estavel ? "LIGADA" : "DESLIGADA");
        }

        k_msleep(T_LEITURA_ENTRADAS_MS);
    }
}

K_THREAD_DEFINE(entradas, PILHA_THREAD, entradas_thread,
                NULL, NULL, NULL, PRIO_ENTRADAS, 0, 0);

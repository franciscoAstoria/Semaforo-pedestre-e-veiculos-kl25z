/*
 * sinc.c - definicao dos mutex/semaforos e funcoes de modo
 */
#include <zephyr/kernel.h>

#include "config.h"
#include "hw.h"
#include "sinc.h"

/* Os tempos precisam "fechar" para os dois semaforos ficarem em sincronia */
BUILD_ASSERT(T_CARRO_VERMELHO_MS == T_PED_VERDE_MS,
             "carro vermelho deve durar o mesmo que pedestre verde");
BUILD_ASSERT(T_PED_VERMELHO_MS == T_CARRO_VERDE_MS + T_CARRO_AMARELO_MS,
             "pedestre vermelho deve durar carro verde + amarelo");

K_MUTEX_DEFINE(mutex_carro);
K_MUTEX_DEFINE(mutex_pedestre);

K_SEM_DEFINE(vez_carro_verde, 0, 1);
K_SEM_DEFINE(vez_carro_amarelo, 0, 1);
K_SEM_DEFINE(vez_carro_vermelho, 0, 1);
K_SEM_DEFINE(vez_ped_verde, 0, 1);
K_SEM_DEFINE(vez_ped_vermelho, 0, 1);

K_SEM_DEFINE(libera_pedestre, 0, 1);
K_SEM_DEFINE(pedestres_parados, 0, 1);

volatile modo_t modo = MODO_DIA;
volatile bool pedido_noturno;
int64_t t0_noturno;

K_SEM_DEFINE(botao_travessia, 0, 1);
volatile bool carro_verde_aceso;

/* Dorme ate um instante absoluto (evita acumular atraso no pisca). */
void dormir_ate(int64_t instante_ms)
{
    int64_t falta = instante_ms - k_uptime_get();

    if (falta > 0) {
        k_msleep((int32_t)falta);
    }
}

/* Chamada SEMPRE no fim de um ciclo (carro vermelho acabou de apagar e o
 * pedestre esta vermelho fixo): e o ponto seguro para trocar de modo.
 * Quem chama ainda precisa acordar o lado dos carros. */
void entra_modo_noturno(void)
{
    modo = MODO_NOTURNO;
    t0_noturno = k_uptime_get() + 20; /* mesma referencia para os 2 piscas */
    log_evento("MODO NOTURNO");
    k_sem_give(&libera_pedestre);     /* pedestre vermelho comeca a piscar */
}

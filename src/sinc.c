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

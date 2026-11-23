/*
 * sinc.h - objetos de sincronizacao compartilhados entre as threads
 *
 * Dentro de cada semaforo:
 *   - um MUTEX garante que so um LED daquele semaforo fica aceso;
 *   - semaforos "vez_*" passam o bastao de um LED para o proximo.
 * Entre os dois semaforos:
 *   - libera_pedestre:   carro vermelho aceso -> pedestre pode abrir
 *   - pedestres_parados: pedestre vermelho aceso -> carro pode seguir
 */
#ifndef SEMAFORO_SINC_H
#define SEMAFORO_SINC_H

#include <zephyr/kernel.h>
#include <stdbool.h>

extern struct k_mutex mutex_carro;
extern struct k_mutex mutex_pedestre;

extern struct k_sem vez_carro_verde;
extern struct k_sem vez_carro_amarelo;
extern struct k_sem vez_carro_vermelho;
extern struct k_sem vez_ped_verde;
extern struct k_sem vez_ped_vermelho;

extern struct k_sem libera_pedestre;
extern struct k_sem pedestres_parados;

typedef enum { MODO_DIA = 0, MODO_NOTURNO = 1 } modo_t;

extern volatile modo_t modo;           /* modo atual do sistema            */
extern volatile bool   pedido_noturno; /* estado (filtrado) da chave       */
extern int64_t         t0_noturno;     /* instante de referencia do pisca  */

void entra_modo_noturno(void);
void dormir_ate(int64_t instante_ms);

extern struct k_sem  botao_travessia;   /* entradas -> carro verde          */
extern volatile bool carro_verde_aceso; /* botao so vale com carro verde    */

#endif /* SEMAFORO_SINC_H */

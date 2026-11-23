/*
 * main.c - inicializacao
 * As threads sao criadas automaticamente por K_THREAD_DEFINE; o main so
 * configura o hardware e da a partida em estado seguro.
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "config.h"
#include "hw.h"
#include "sinc.h"

int main(void)
{
    if (hw_init() != 0) {
        return 0;
    }

    printk("# Semaforos de veiculos e pedestres - Zephyr / FRDM-KL25Z\n");
    printk("# MODO_TESTE=%d (0=completo, 1=so carro, 2=so pedestre)\n",
           MODO_TESTE);

    /* Partida segura: pedestre vermelho ANTES de qualquer verde p/ carros */
    k_sem_give(&vez_ped_vermelho);
    k_sem_take(&pedestres_parados, K_FOREVER);
    k_sem_give(&vez_carro_verde);

    return 0;
}

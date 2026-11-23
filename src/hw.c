/*
 * hw.c - acesso aos pinos, log e verificacao de seguranca em tempo real
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <errno.h>
#include <stdarg.h>

#include "config.h"
#include "hw.h"

static const struct device *const gpio_a = DEVICE_DT_GET(DT_NODELABEL(gpioa));
static const struct device *const gpio_b = DEVICE_DT_GET(DT_NODELABEL(gpiob));
static const struct device *const gpio_d = DEVICE_DT_GET(DT_NODELABEL(gpiod));

static const char *const nome_led[NUM_LEDS] = {
    [CARRO_VERDE]    = "CARRO_VERDE",
    [CARRO_AMARELO]  = "CARRO_AMARELO",
    [CARRO_VERMELHO] = "CARRO_VERMELHO",
    [PED_VERDE]      = "PED_VERDE",
    [PED_VERMELHO]   = "PED_VERMELHO",
};

static bool aceso_agora[NUM_LEDS];
static bool pronto;

/* Serializa escrita nos pinos + impressao no log, para que a ordem das
 * linhas do log seja exatamente a ordem real das mudancas nos LEDs. */
static K_MUTEX_DEFINE(mutex_saida);

static void escreve_pinos(led_t led, bool aceso)
{
    switch (led) {
    case CARRO_VERDE:     /* RGB: nivel 0 acende */
        gpio_pin_set_raw(gpio_b, PINO_RGB_VERDE, aceso ? 0 : 1);
        break;
    case CARRO_VERMELHO:
        gpio_pin_set_raw(gpio_b, PINO_RGB_VERMELHO, aceso ? 0 : 1);
        break;
    case CARRO_AMARELO:   /* amarelo = vermelho + verde */
        gpio_pin_set_raw(gpio_b, PINO_RGB_VERMELHO, aceso ? 0 : 1);
        gpio_pin_set_raw(gpio_b, PINO_RGB_VERDE, aceso ? 0 : 1);
        break;
    case PED_VERDE:       /* LED externo: nivel 1 acende */
        gpio_pin_set_raw(gpio_d, PINO_PED_VERDE, aceso ? 1 : 0);
        break;
    case PED_VERMELHO:
        gpio_pin_set_raw(gpio_b, PINO_PED_VERMELHO, aceso ? 1 : 0);
        break;
    default:
        break;
    }
}

static void erro_seguranca(const char *msg)
{
    printk("[%u ms] ERRO %s\n", k_uptime_get_32(), msg);
}

/* Chamada a cada mudanca de LED (com mutex_saida travado). */
static void verifica_seguranca(void)
{
    int carros = aceso_agora[CARRO_VERDE] + aceso_agora[CARRO_AMARELO] +
                 aceso_agora[CARRO_VERMELHO];
    int pedestres = aceso_agora[PED_VERDE] + aceso_agora[PED_VERMELHO];

    if (carros > 1) {
        erro_seguranca("dois LEDs do semaforo de carros acesos ao mesmo tempo");
    }
    if (pedestres > 1) {
        erro_seguranca("dois LEDs do semaforo de pedestres acesos ao mesmo tempo");
    }
#if MODO_TESTE == MODO_COMPLETO
    if (aceso_agora[PED_VERDE] && !aceso_agora[CARRO_VERMELHO]) {
        erro_seguranca("pedestre VERDE sem carro VERMELHO");
    }
    if (aceso_agora[CARRO_VERDE] && !aceso_agora[PED_VERMELHO]) {
        erro_seguranca("carro VERDE sem pedestre VERMELHO");
    }
#endif
}

void led_set(led_t led, bool aceso)
{
    k_mutex_lock(&mutex_saida, K_FOREVER);
    escreve_pinos(led, aceso);
    aceso_agora[led] = aceso;
    printk("[%u ms] LED %s %s\n", k_uptime_get_32(), nome_led[led],
           aceso ? "ON" : "OFF");
    verifica_seguranca();
    k_mutex_unlock(&mutex_saida);
}

void log_evento(const char *fmt, ...)
{
    va_list ap;

    k_mutex_lock(&mutex_saida, K_FOREVER);
    printk("[%u ms] EVT ", k_uptime_get_32());
    va_start(ap, fmt);
    vprintk(fmt, ap);
    va_end(ap);
    printk("\n");
    k_mutex_unlock(&mutex_saida);
}

int hw_init(void)
{
    if (!device_is_ready(gpio_b) || !device_is_ready(gpio_d)
        || !device_is_ready(gpio_a)
        ) {
        printk("# ERRO: GPIO nao esta pronto\n");
        return -ENODEV;
    }

    /* todos os LEDs comecam apagados */
    gpio_pin_configure(gpio_b, PINO_RGB_VERMELHO, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio_b, PINO_RGB_VERDE, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio_b, PINO_PED_VERMELHO, GPIO_OUTPUT_LOW);
    gpio_pin_configure(gpio_d, PINO_PED_VERDE, GPIO_OUTPUT_LOW);
    gpio_pin_configure(gpio_a, PINO_CHAVE_NOTURNO, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio_d, PINO_BOTAO, GPIO_INPUT | GPIO_PULL_UP);

    pronto = true;
    return 0;
}

bool hw_pronto(void)
{
    return pronto;
}

bool hw_chave_noturno(void)
{
    return gpio_pin_get_raw(gpio_a, PINO_CHAVE_NOTURNO) == 0;
}

bool hw_botao(void)
{
    return gpio_pin_get_raw(gpio_d, PINO_BOTAO) == 0;
}

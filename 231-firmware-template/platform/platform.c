#include "platform/platform.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BUTTON_PIN 15

void platform_init(void)
{
    stdio_init_all();

#ifdef PICO_DEFAULT_LED_PIN
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
#endif

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
}

uint64_t platform_time_us(void)
{
    return time_us_64();
}

void platform_led_set(bool on)
{
#ifdef PICO_DEFAULT_LED_PIN
    gpio_put(PICO_DEFAULT_LED_PIN, on ? 1 : 0);
#else
    (void)on;
#endif
}

bool platform_button_read(void)
{
    /* Кнопка замыкает GP15 на землю (активный уровень — 0) */
    return !gpio_get(BUTTON_PIN);
}

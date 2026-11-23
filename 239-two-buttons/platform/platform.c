#include "platform/platform.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BUTTON_1_PIN 15
#define BUTTON_2_PIN 14

void platform_init(void)
{
    stdio_init_all();

#ifdef PICO_DEFAULT_LED_PIN
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
#endif

    gpio_init(BUTTON_1_PIN);
    gpio_set_dir(BUTTON_1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_1_PIN);

    gpio_init(BUTTON_2_PIN);
    gpio_set_dir(BUTTON_2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_2_PIN);
}

void platform_led_set(bool on)
{
#ifdef PICO_DEFAULT_LED_PIN
    gpio_put(PICO_DEFAULT_LED_PIN, on ? 1 : 0);
#else
    (void)on;
#endif
}

bool platform_button_read(platform_button_t button)
{
    switch (button)
    {
        case PLATFORM_BUTTON_1:
            return !gpio_get(BUTTON_1_PIN);
        case PLATFORM_BUTTON_2:
            return !gpio_get(BUTTON_2_PIN);
        default:
            return false;
    }
}

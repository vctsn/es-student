#include "led-task/led-task.h"
#include "platform/platform.h"
#include "systime/systime.h"
#include <stdbool.h>

#define LED_DEFAULT_PERIOD_MS 1000U

static led_state_t s_state = LED_STATE_BLINK;
static uint32_t s_period_ms = LED_DEFAULT_PERIOD_MS;
static bool s_level = false;
static uint64_t s_last_us = 0;

void led_task_init(void)
{
    s_period_ms = LED_DEFAULT_PERIOD_MS;
    led_task_set_state(LED_STATE_BLINK);
}

void led_task_set_state(led_state_t state)
{
    s_state = state;
    if (s_state == LED_STATE_OFF)
    {
        s_level = false;
        platform_led_set(false);
    }
    else if (s_state == LED_STATE_ON)
    {
        s_level = true;
        platform_led_set(true);
    }
    else if (s_state == LED_STATE_BLINK)
    {
        uint64_t half_period_us = ((uint64_t)s_period_ms * 1000ULL) / 2ULL;
        s_last_us = systime_us() - half_period_us;
    }
}

led_state_t led_task_get_state(void)
{
    return s_state;
}

void led_task_set_period_ms(uint32_t period_ms)
{
    if (period_ms == 0)
    {
        return;
    }
    s_period_ms = period_ms;
}

uint32_t led_task_get_period_ms(void)
{
    return s_period_ms;
}

void led_task_next_state(void)
{
    switch (s_state)
    {
        case LED_STATE_OFF:
            led_task_set_state(LED_STATE_ON);
            break;

        case LED_STATE_ON:
            led_task_set_state(LED_STATE_BLINK);
            break;

        case LED_STATE_BLINK:
            led_task_set_state(LED_STATE_OFF);
            break;
    }
}

void led_task_handle(void)
{
    switch (s_state)
    {
        case LED_STATE_OFF:
        case LED_STATE_ON:
            break;

        case LED_STATE_BLINK:
        {
            uint64_t half_period_us = ((uint64_t)s_period_ms * 1000ULL) / 2ULL;
            if (systime_period_elapsed(&s_last_us, half_period_us))
            {
                s_level = !s_level;
                platform_led_set(s_level);
            }
            break;
        }
    }
}

void led_task_next_period(void)
{
    if (s_period_ms == 250)
    {
        led_task_set_period_ms(500);
    }
    else if (s_period_ms == 500)
    {
        led_task_set_period_ms(1000);
    }
    else
    {
        led_task_set_period_ms(250);
    }
}

void led_task_blink(void)
{
    led_task_set_state(LED_STATE_BLINK);
}

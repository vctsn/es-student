#include "button-task/button-task.h"
#include "platform/platform.h"
#include <stddef.h>

#define BUTTON_DEBOUNCE_US 20000ULL

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_DEBOUNCE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_DEBOUNCE,
} button_state_t;

static button_state_t s_state = BUTTON_STATE_RELEASED;
static uint64_t s_debounce_start_us = 0;
static uint32_t s_press_count = 0;
static button_task_callback_t s_on_press = NULL;

void button_task_init(button_task_callback_t on_press)
{
    s_on_press = on_press;
    s_state = BUTTON_STATE_RELEASED;
    s_debounce_start_us = 0;
    s_press_count = 0;
}

void button_task_handle(void)
{
    bool raw_pressed = platform_button_read();
    uint64_t now_us = platform_time_us();

    switch (s_state)
    {
        case BUTTON_STATE_RELEASED:
            if (raw_pressed)
            {
                s_debounce_start_us = now_us;
                s_state = BUTTON_STATE_PRESS_DEBOUNCE;
            }
            break;

        case BUTTON_STATE_PRESS_DEBOUNCE:
            if (!raw_pressed)
            {
                s_state = BUTTON_STATE_RELEASED;
            }
            else if ((now_us - s_debounce_start_us) >= BUTTON_DEBOUNCE_US)
            {
                s_state = BUTTON_STATE_PRESSED;
                s_press_count++;
                if (s_on_press != NULL)
                {
                    s_on_press();
                }
            }
            break;

        case BUTTON_STATE_PRESSED:
            if (!raw_pressed)
            {
                s_debounce_start_us = now_us;
                s_state = BUTTON_STATE_RELEASE_DEBOUNCE;
            }
            break;

        case BUTTON_STATE_RELEASE_DEBOUNCE:
            if (raw_pressed)
            {
                s_state = BUTTON_STATE_PRESSED;
            }
            else if ((now_us - s_debounce_start_us) >= BUTTON_DEBOUNCE_US)
            {
                s_state = BUTTON_STATE_RELEASED;
            }
            break;
    }
}

bool button_task_is_pressed(void)
{
    return (s_state == BUTTON_STATE_PRESSED || s_state == BUTTON_STATE_RELEASE_DEBOUNCE);
}

uint32_t button_task_get_press_count(void)
{
    return s_press_count;
}

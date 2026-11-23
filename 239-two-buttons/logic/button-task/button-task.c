#include "button-task/button-task.h"
#include "systime/systime.h"
#include <stddef.h>

static const uint64_t BUTTON_DEBOUNCE_US = 20000;
static const uint64_t BUTTON_LONG_PRESS_US = 1000000;

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_DEBOUNCE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_DEBOUNCE,
} button_state_t;

typedef struct {
    button_state_t state;
    uint64_t debounce_start_us;
    uint64_t press_start_us;
    uint32_t press_count;
    bool long_reported;
    button_task_callback_t on_press;
    button_task_callback_t on_long_press;
} button_ctx_t;

static button_ctx_t s_buttons[PLATFORM_BUTTON_COUNT];

void button_task_init(platform_button_t button,
                      button_task_callback_t on_press,
                      button_task_callback_t on_long_press)
{
    if (button >= PLATFORM_BUTTON_COUNT)
    {
        return;
    }

    s_buttons[button].state = BUTTON_STATE_RELEASED;
    s_buttons[button].debounce_start_us = 0;
    s_buttons[button].press_start_us = 0;
    s_buttons[button].press_count = 0;
    s_buttons[button].long_reported = false;
    s_buttons[button].on_press = on_press;
    s_buttons[button].on_long_press = on_long_press;
}

static void handle_single_button(platform_button_t id)
{
    button_ctx_t *btn = &s_buttons[id];
    bool raw_pressed = platform_button_read(id);
    uint64_t now_us = systime_us();

    switch (btn->state)
    {
        case BUTTON_STATE_RELEASED:
            if (raw_pressed)
            {
                btn->debounce_start_us = now_us;
                btn->state = BUTTON_STATE_PRESS_DEBOUNCE;
            }
            break;

        case BUTTON_STATE_PRESS_DEBOUNCE:
            if (!raw_pressed)
            {
                btn->state = BUTTON_STATE_RELEASED;
            }
            else if ((now_us - btn->debounce_start_us) >= BUTTON_DEBOUNCE_US)
            {
                btn->state = BUTTON_STATE_PRESSED;
                btn->press_start_us = now_us;
                btn->long_reported = false;
                btn->press_count++;

                if (btn->on_long_press == NULL && btn->on_press != NULL)
                {
                    btn->on_press();
                }
            }
            break;

        case BUTTON_STATE_PRESSED:
            if (!raw_pressed)
            {
                btn->debounce_start_us = now_us;
                btn->state = BUTTON_STATE_RELEASE_DEBOUNCE;
            }
            else if (btn->on_long_press != NULL &&
                     !btn->long_reported &&
                     (now_us - btn->press_start_us) >= BUTTON_LONG_PRESS_US)
            {
                btn->long_reported = true;
                btn->on_long_press();
            }
            break;

        case BUTTON_STATE_RELEASE_DEBOUNCE:
            if (raw_pressed)
            {
                btn->state = BUTTON_STATE_PRESSED;
            }
            else if ((now_us - btn->debounce_start_us) >= BUTTON_DEBOUNCE_US)
            {
                btn->state = BUTTON_STATE_RELEASED;
                if (btn->on_long_press != NULL && !btn->long_reported && btn->on_press != NULL)
                {
                    btn->on_press();
                }
            }
            break;
    }
}

void button_task_handle(void)
{
    for (int i = 0; i < PLATFORM_BUTTON_COUNT; i++)
    {
        handle_single_button((platform_button_t)i);
    }
}

bool button_task_is_pressed(platform_button_t button)
{
    if (button >= PLATFORM_BUTTON_COUNT)
    {
        return false;
    }
    button_state_t st = s_buttons[button].state;
    return (st == BUTTON_STATE_PRESSED || st == BUTTON_STATE_RELEASE_DEBOUNCE);
}

uint32_t button_task_get_press_count(platform_button_t button)
{
    if (button >= PLATFORM_BUTTON_COUNT)
    {
        return 0;
    }
    return s_buttons[button].press_count;
}

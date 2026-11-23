#include "platform/platform.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include "api/api.h"
#include "led-task/led-task.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"
#include <stddef.h>

int main(void)
{
    platform_init();
    stdio_text_protocol_init();
    led_task_init();
    button_task_init(led_task_next_state);
    pi_task_init();

    while (1)
    {
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
        {
            api_handle(command);
        }

        led_task_handle();
        button_task_handle();
        pi_task_handle();
    }

    return 0;
}



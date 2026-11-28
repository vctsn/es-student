#include "block-task.h"
#include "platform/platform.h"
#include <stddef.h>

static uint32_t task_count = 0;
static block_task_callback_t task_callback = NULL;

bool block_task_start(uint32_t count, block_task_callback_t on_block)
{
    if (task_callback != NULL || on_block == NULL)
    {
        return false;
    }

    if (!platform_block_start(count))
    {
        return false;
    }

    task_count = count;
    task_callback = on_block;
    return true;
}

void block_task_handle(void)
{
    if (task_callback == NULL || !platform_block_done())
    {
        return;
    }

    block_task_callback_t cb = task_callback;
    uint32_t count = task_count;

    // забываем функцию ДО вызова, так как она сама может запустить новый захват
    task_callback = NULL;
    task_count = 0;

    cb(platform_block_samples(), count, platform_block_time_us());
}

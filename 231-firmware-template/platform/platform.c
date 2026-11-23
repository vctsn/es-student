#include "platform/platform.h"
#include "pico/stdlib.h"

void platform_init(void)
{
    stdio_init_all();
}

uint64_t platform_time_us(void)
{
    return time_us_64();
}

#include "systime/systime.h"
#include "pico/time.h"

uint64_t systime_us(void)
{
    return to_us_since_boot(get_absolute_time());
}

uint32_t systime_ms(void)
{
    return to_ms_since_boot(get_absolute_time());
}

bool systime_period_elapsed(uint64_t *last_us, uint64_t period_us)
{
    if (period_us == 0)
    {
        return true;
    }

    uint64_t now = systime_us();
    uint64_t elapsed = now - *last_us;

    if (elapsed < period_us)
    {
        return false;
    }

    if (elapsed >= 2 * period_us)
    {
        *last_us = now;
    }
    else
    {
        *last_us += period_us;
    }

    return true;
}

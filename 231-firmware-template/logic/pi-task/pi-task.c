#include "pi-task/pi-task.h"
#include "platform/platform.h"
#include <stddef.h>

/* 1 член ряда ≈ 585 тактов при 125 МГц = 4.68 мкс; 100 членов ≈ 0.47 мс (< 0.5 мс) */
static const uint32_t PI_TERMS_PER_CALL = 100;

static pi_state_t s_state = PI_STATE_NOT_STARTED;
static uint32_t s_total_terms = 0;
static uint32_t s_current_term = 0;
static double s_sum = 0.0;
static double s_result = 0.0;
static uint64_t s_start_us = 0;
static uint32_t s_elapsed_ms = 0;

void pi_task_init(void)
{
    s_state = PI_STATE_NOT_STARTED;
    s_total_terms = 0;
    s_current_term = 0;
    s_sum = 0.0;
    s_result = 0.0;
    s_start_us = 0;
    s_elapsed_ms = 0;
}

bool pi_task_start(uint32_t terms)
{
    if (s_state == PI_STATE_RUNNING || terms == 0)
    {
        return false;
    }

    s_total_terms = terms;
    s_current_term = 0;
    s_sum = 0.0;
    s_start_us = platform_time_us();
    s_state = PI_STATE_RUNNING;
    return true;
}

void pi_task_handle(void)
{
    if (s_state != PI_STATE_RUNNING)
    {
        return;
    }

    uint32_t end_term = s_current_term + PI_TERMS_PER_CALL;
    if (end_term > s_total_terms || end_term < s_current_term)
    {
        end_term = s_total_terms;
    }

    for (uint32_t k = s_current_term; k < end_term; k++)
    {
        double denom = 2.0 * (double)k + 1.0;
        if ((k & 1U) == 0U)
        {
            s_sum += 1.0 / denom;
        }
        else
        {
            s_sum -= 1.0 / denom;
        }
    }

    s_current_term = end_term;

    if (s_current_term >= s_total_terms)
    {
        s_result = 4.0 * s_sum;
        s_elapsed_ms = (uint32_t)((platform_time_us() - s_start_us) / 1000ULL);
        s_state = PI_STATE_DONE;
    }
}

pi_state_t pi_task_get_state(void)
{
    return s_state;
}

double pi_task_get_result(uint32_t *elapsed_ms)
{
    if (elapsed_ms != NULL)
    {
        *elapsed_ms = s_elapsed_ms;
    }
    return s_result;
}

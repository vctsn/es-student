#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifndef PROFILING_ENABLED
#define PROFILING_ENABLED 1
#endif

#define PROFILING_MAX_STOPWATCHES 8

/* Сумма замеров хранится в uint32_t: переполняется примерно через 71 минуту (4294 с) непрерывной работы секундомера */
typedef struct
{
    const char *name;
    uint32_t count;
    double mean_us;
    uint32_t max_us;
} profiling_result_t;

void profiling_stopwatch_init(uint32_t id, const char *name);
bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);

#if PROFILING_ENABLED
void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);
#else
#define profiling_start(id) ((void)0)
#define profiling_stop(id) ((void)0)
#endif


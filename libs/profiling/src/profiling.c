#include "profiling/profiling.h"
#include "systime/systime.h"
#include <stddef.h>

typedef struct
{
    const char *name;
    uint32_t start_us;
    uint32_t total_us;
    uint32_t count;
    uint32_t max_us;
    bool initialized;
} stopwatch_t;

static stopwatch_t s_stopwatches[PROFILING_MAX_STOPWATCHES];

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    if (id >= PROFILING_MAX_STOPWATCHES)
    {
        return;
    }
    s_stopwatches[id].name = name;
    s_stopwatches[id].start_us = 0;
    s_stopwatches[id].total_us = 0;
    s_stopwatches[id].count = 0;
    s_stopwatches[id].max_us = 0;
    s_stopwatches[id].initialized = true;
}

void profiling_start(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES || !s_stopwatches[id].initialized)
    {
        return;
    }
    s_stopwatches[id].start_us = (uint32_t)systime_us();
}

void profiling_stop(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES || !s_stopwatches[id].initialized)
    {
        return;
    }
    uint32_t dt = (uint32_t)systime_us() - s_stopwatches[id].start_us;
    s_stopwatches[id].total_us += dt;
    s_stopwatches[id].count++;
    if (dt > s_stopwatches[id].max_us)
    {
        s_stopwatches[id].max_us = dt;
    }
}

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if (id >= PROFILING_MAX_STOPWATCHES || !s_stopwatches[id].initialized || result == NULL)
    {
        return false;
    }
    const stopwatch_t *sw = &s_stopwatches[id];
    result->name = sw->name;
    result->count = sw->count;
    result->mean_us = (sw->count > 0) ? ((double)sw->total_us / (double)sw->count) : 0.0;
    result->max_us = sw->max_us;
    return true;
}

void profiling_reset(void)
{
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++)
    {
        s_stopwatches[i].total_us = 0;
        s_stopwatches[i].count = 0;
        s_stopwatches[i].max_us = 0;
    }
}

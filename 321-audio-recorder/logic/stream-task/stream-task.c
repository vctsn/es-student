#include "stream-task/stream-task.h"
#include "platform/platform.h"
#include <stddef.h>

static stream_task_callback_t frame_callback = NULL;
static stream_state_t stream_state = STREAM_STATE_IDLE;
static uint8_t stream_bits = 16;
static uint16_t frame_number = 0;
static uint32_t total_frames = 0;
static uint32_t total_samples = 0;

void stream_task_init(stream_task_callback_t on_frame)
{
    frame_callback = on_frame;
    stream_state = STREAM_STATE_IDLE;
}

bool stream_task_start(uint8_t bits)
{
    if (stream_state == STREAM_STATE_RUN || !platform_stream_start(bits))
    {
        return false;
    }

    stream_bits = bits;
    frame_number = 0;
    total_frames = 0;
    total_samples = 0;
    stream_state = STREAM_STATE_RUN;
    return true;
}

void stream_task_stop(void)
{
    if (stream_state != STREAM_STATE_RUN)
    {
        return;
    }

    platform_stream_stop();
    stream_state = STREAM_STATE_IDLE;
}

void stream_task_handle(void)
{
    if (stream_state != STREAM_STATE_RUN || frame_callback == NULL)
    {
        return;
    }

    uint32_t samples = 0;
    const uint8_t *buf = platform_stream_next(&samples);
    if (buf == NULL)
    {
        return;
    }

    stream_header_t header = {
        .sync_word = STREAM_SYNC_WORD,
        .number = frame_number,
        .count = (uint16_t)samples,
    };

    uint32_t bytes = (stream_bits == 16) ? (samples * 2U) : samples;
    frame_callback(&header, buf, bytes);
    platform_stream_release();

    frame_number++;
    total_frames++;
    total_samples += samples;
}

stream_state_t stream_task_get_state(void)
{
    return stream_state;
}

uint8_t stream_task_get_bits(void)
{
    return stream_bits;
}

uint32_t stream_task_get_frames(void)
{
    return total_frames;
}

uint32_t stream_task_get_samples(void)
{
    return total_samples;
}

uint32_t stream_task_get_overruns(void)
{
    return platform_stream_overruns();
}

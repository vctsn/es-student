#include "api/api-commands.h"
#include "firmware.h"
#include "device/device.h"
#include "systime/systime.h"
#include "profiling/profiling.h"
#include "led-task/led-task.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"
#include "telemetry-task/telemetry-task.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "platform/platform.h"
#include "block-task/block-task.h"
#include "stream-task/stream-task.h"


#define TELEMETRY_DEFAULT_PERIOD_MS 100
#define STREAM_BLINK_PERIOD_MS 250



static bool parse_u32(const char *text, uint32_t *out)
{
    if (text == NULL || *text == '\0')
    {
        return false;
    }

    for (const char *p = text; *p != '\0'; p++)
    {
        if (*p < '0' || *p > '9')
        {
            return false;
        }
    }

    errno = 0;
    char *end = NULL;
    unsigned long val = strtoul(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || val > 4294967295UL)
    {
        return false;
    }

    *out = (uint32_t)val;
    return true;
}


void api_send_frame(const stream_header_t *header, const uint8_t *data, uint32_t bytes)
{
    platform_write(header, sizeof(*header));
    platform_write(data, bytes);
}

static void cmd_stream_start(const command_t *command)
{
    uint32_t bits = 16;

    if (command->argc > 1)
    {
        printf("error: usage stream_start [8|16]\n");
        return;
    }

    if (command->argc == 1)
    {
        if (!parse_u32(command->argv[0], &bits) || (bits != 8 && bits != 16))
        {
            printf("error: usage stream_start [8|16]\n");
            return;
        }
    }

    if (telemetry_task_get_state() == TELEMETRY_STATE_RUN || !stream_task_start((uint8_t)bits))
    {
        printf("error: adc busy\n");
        return;
    }

    printf("stream: started, 44100 Hz, %lu bit\n", (unsigned long)bits);
    led_task_set_state(LED_STATE_OFF);
    platform_blink_start(STREAM_BLINK_PERIOD_MS);
}

static void cmd_stream_stop(const command_t *command)
{
    (void)command;
    if (stream_task_get_state() == STREAM_STATE_RUN)
    {
        stream_task_stop();
        platform_blink_stop();
    }
    printf("stream: stopped, %lu frames, %lu samples, %lu overruns\n",
           (unsigned long)stream_task_get_frames(),
           (unsigned long)stream_task_get_samples(),
           (unsigned long)stream_task_get_overruns());
}


static void print_dma_cap(const uint16_t *samples, uint32_t count, uint32_t time_us)
{
    if (samples == NULL || count == 0)
    {
        return;
    }

    uint16_t min_val = samples[0];
    uint16_t max_val = samples[0];
    uint64_t sum = 0;

    for (uint32_t i = 0; i < count; i++)
    {
        uint16_t val = samples[i];
        if (val < min_val)
        {
            min_val = val;
        }
        if (val > max_val)
        {
            max_val = val;
        }
        sum += val;
    }

    double mean_val = (double)sum / (double)count;
    uint64_t expected_us = (uint64_t)count * 1000000ULL / PLATFORM_SAMPLE_RATE_HZ;

    printf("dma_cap: %lu samples in %lu us, expected %llu us, min %u max %u mean %.1f\n",
           (unsigned long)count,
           (unsigned long)time_us,
           (unsigned long long)expected_us,
           min_val,
           max_val,
           mean_val);
}

static void cmd_dma_cap(const command_t *command)
{
    uint32_t count = 0;
    if (command->argc != 1 || !parse_u32(command->argv[0], &count))
    {
        printf("error: usage dma_cap <N>\n");
        return;
    }
    if (count == 0 || count > PLATFORM_BLOCK_MAX)
    {
        printf("error: N must be 1..8192\n");
        return;
    }
    if (telemetry_task_get_state() == TELEMETRY_STATE_RUN || !block_task_start(count, print_dma_cap))
    {
        printf("error: adc busy\n");
        return;
    }
}


static void cmd_cap_start(const command_t *command)
{
    uint32_t period = 0;
    uint32_t capture_count = 0;
    if (command->argc != 2 || !parse_u32(command->argv[0], &capture_count) || !parse_u32(command->argv[1], &period))
    {
        printf("error: usage cap_start <N> <period_us>\n");
        return;
    }
    if (capture_count == 0 || capture_count > PLATFORM_CAPTURE_MAX || period < 10)
    {
        printf("error: N must be 1..2000, period_us at least 10\n");
        return;
    }
    if (!platform_capture_start(capture_count, period) || telemetry_task_get_state() == TELEMETRY_STATE_RUN)
    {
        printf("error: adc busy\n");
    }

    printf("capture: started, %ld samples, period %ld us\n", capture_count, period);
}

static void cmd_cap_dump(const command_t *command)
{
    if (!platform_capture_done())
    {
        printf("error: no capture\n");
        return;
    }
    platform_capture_t cap;
    platform_capture_get(&cap);

    printf("capture: %lu samples, period %lu us, interval %lu..%lu us, handler %.1f %%\n",
           (unsigned long)cap.count,
           (unsigned long)cap.period_us,
           (unsigned long)cap.min_interval_us,
           (unsigned long)cap.max_interval_us,
           (double)cap.handler_percent);
        
    for (uint32_t i = 0; i < cap.count; i++)
    {
        if (i % 16 == 0)
        {
            printf("cap:");
        }
        printf(" %u", cap.samples[i]);
        if (i % 16 == 15 || i == cap.count - 1)
        {
            printf("\n");
        }
    }

}

static void cmd_timer_blink_start(const command_t *command)
{
    uint32_t period = 0;
    if (command->argc != 1 || !parse_u32(command->argv[0], &period))
    {
        printf("error: usage timer_blink_start <period_ms>\n");
        return;
    }
    if (!platform_blink_start(period))
    {
        printf("error: period_ms must be 10..60000\n");
        return;
    }
    printf("timer_blink: started, period %lu ms\n", (unsigned long)period);
}

static void cmd_timer_blink_stop(const command_t *command)
{
    (void)command;
    uint32_t toggles = platform_blink_stop();
    printf("timer_blink: stopped, %lu toggles\n", (unsigned long)toggles);
}


static const char *led_state_name(led_state_t state)
{
    switch (state)
    {
        case LED_STATE_OFF:   return "off";
        case LED_STATE_ON:    return "on";
        case LED_STATE_BLINK: return "blink";
        default:              return "unknown";
    }
}

void api_print_telemetry(const telemetry_record_t *record)
{
    if (record == NULL)
    {
        return;
    }
    printf("tm: %llu %.3f %.1f\r\n",
           (unsigned long long)record->time_us,
           record->voltage_v,
           record->temp_c);
}

static void cmd_tm_start(const command_t *command)
{
    if (platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
    uint32_t period_ms = TELEMETRY_DEFAULT_PERIOD_MS;

    if (command->argc > 1)
    {
        printf("error: usage tm_start [<period_ms>]\r\n");
        return;
    }

    if (command->argc == 1)
    {
        if (!parse_u32(*command->argv, &period_ms))
        {
            printf("error: usage tm_start [<period_ms>]\r\n");
            return;
        }
    }

    if (!telemetry_task_start(period_ms))
    {
        printf("error: period_ms must be at least 10\r\n");
        return;
    }

    printf("telemetry: started, period %lu ms\r\n", (unsigned long)period_ms);
}

static void cmd_tm_stop(const command_t *command)
{
    (void)command;
    telemetry_task_stop();
    printf("telemetry: stopped\r\n");
}

static void print_led_status(void)
{
    printf("led: %s, period %lu ms\r\n",
           led_state_name(led_task_get_state()),
           (unsigned long)led_task_get_period_ms());
}

static void cmd_get_voltage(const command_t *command)
{
    if (platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
    (void)command;
    float voltage = platform_voltage_read();
    printf("voltage: %.3f V\n", voltage);
}

static void cmd_get_temp(const command_t *command)
{
    if (platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
    (void)command;
    float temp = platform_temp_read();
    printf("temp: %.1f C\n", temp);
}

static void cmd_info(const command_t *command)
{
    (void)command;
    device_info_t info;
    device_get_info(&info);

    printf("name: %s\r\n", FIRMWARE_NAME);
    printf("version: %s\r\n", FIRMWARE_VERSION);
    printf("project: %s\r\n", FIRMWARE_PROJECT);
    printf("repo: %s\r\n", FIRMWARE_REPO);
    printf("board: %s\r\n", info.board);
    printf("serial: %s\r\n", info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\r\n",
           (unsigned int)info.chip_manufacturer,
           (unsigned int)info.chip_part,
           (unsigned int)info.chip_revision);
    printf("pico-sdk: %s\r\n", info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    (void)command;
    printf("uptime: %llu ms\r\n", (unsigned long long)systime_ms());
}

static void cmd_led_enable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_ON);
    print_led_status();
}

static void cmd_led_disable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_OFF);
    print_led_status();
}

static void cmd_led_blink(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_BLINK);
    print_led_status();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms = 0;
    if (command->argc != 1 || !parse_u32(*command->argv, &period_ms))
    {
        printf("error: usage led_period <period_ms>\r\n");
        return;
    }

    if (period_ms == 0)
    {
        printf("error: period_ms must be greater than 0\r\n");
        return;
    }

    led_task_set_period_ms(period_ms);
    print_led_status();
}

static void cmd_button(const command_t *command)
{
    (void)command;
    printf("button: %s, presses %lu\r\n",
           button_task_is_pressed() ? "pressed" : "released",
           (unsigned long)button_task_get_press_count());
}

static void cmd_pi_start(const command_t *command)
{
    uint32_t terms = 1000000U;

    if (command->argc > 1)
    {
        printf("error: usage pi_start [<terms>]\r\n");
        return;
    }

    if (command->argc == 1)
    {
        if (!parse_u32(*command->argv, &terms))
        {
            printf("error: usage pi_start [<terms>]\r\n");
            return;
        }
        if (terms == 0)
        {
            printf("error: terms must be greater than 0\r\n");
            return;
        }
    }

    if (!pi_task_start(terms))
    {
        printf("error: pi is busy\r\n");
        return;
    }

    printf("pi: started, %lu terms\r\n", (unsigned long)terms);
}

static void cmd_pi(const command_t *command)
{
    (void)command;
    pi_state_t state = pi_task_get_state();

    if (state == PI_STATE_NOT_STARTED)
    {
        printf("pi: not started\r\n");
    }
    else if (state == PI_STATE_RUNNING)
    {
        printf("pi: running\r\n");
    }
    else
    {
        uint32_t elapsed_ms = 0;
        double pi = pi_task_get_result(&elapsed_ms);
        printf("pi: %.9f in %lu ms\r\n", pi, (unsigned long)elapsed_ms);
    }
}

static void cmd_charge_on(const command_t *command)
{
    (void)command;
    platform_charge_set(true);
    printf("charge: on\r\n");
}

static void cmd_charge_off(const command_t *command)
{
    (void)command;
    platform_charge_set(false);
    printf("charge: off\r\n");
}

static void cmd_profiling(const command_t *command)
{
    (void)command;
#if PROFILING_ENABLED
    profiling_result_t res;
    for (uint32_t id = 0; id < PROFILING_MAX_STOPWATCHES; id++)
    {
        if (profiling_get(id, &res))
        {
            printf("%-10s count %-10lu mean %.2f us  max %lu us\r\n",
                   res.name,
                   (unsigned long)res.count,
                   res.mean_us,
                   (unsigned long)res.max_us);
        }
    }
#else
    printf("profiling is disabled in this build\r\n");
#endif
}


static void cmd_profiling_reset(const command_t *command)
{
    (void)command;
    profiling_reset();
    printf("profiling reset\r\n");
}


const api_command_t api_commands[] = {
    { "stream_start",    "start audio stream: stream_start [8|16]", cmd_stream_start },
    { "stream_stop",     "stop audio stream",                       cmd_stream_stop },
    { "dma_cap", "capture ADC block via DMA: dma_cap <N>", cmd_dma_cap },
    { "cap_start",       "capture ADC samples: cap_start <N> <period_us>", cmd_cap_start },
    { "cap_dump",        "dump captured ADC samples",                      cmd_cap_dump },
    { "timer_blink_start", "blink led by timer: timer_blink_start <period_ms>", cmd_timer_blink_start },
    { "timer_blink_stop", "stop timer blink, number of toggles", cmd_timer_blink_stop },
    { "charge_on",       "set GP22 high to charge RC",   cmd_charge_on },
    { "charge_off",      "set GP22 low to discharge RC", cmd_charge_off },
    { "tm_start",        "start telemetry stream",         cmd_tm_start },
    { "tm_stop",         "stop telemetry stream",          cmd_tm_stop },
    { "get_temp", "chip temperature, degrees Celsius", cmd_get_temp },
    { "get_voltage", "voltage on GP26, ADC input 0", cmd_get_voltage },
    { "info",            "device passport",                cmd_info },
    { "uptime",          "time since reset",               cmd_uptime },
    { "led_enable",      "turn led on",                    cmd_led_enable },
    { "led_disable",     "turn led off",                   cmd_led_disable },
    { "led_blink",       "blink led",                      cmd_led_blink },
    { "led_period",      "set led blink period",           cmd_led_period },
    { "button",          "button state and press count",   cmd_button },
    { "pi_start",        "start pi calculation",           cmd_pi_start },
    { "pi",              "pi calculation result or state", cmd_pi },
    { "profiling",       "stopwatch measurements",         cmd_profiling },
    { "profiling_reset", "reset stopwatch measurements",   cmd_profiling_reset },
};

const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);


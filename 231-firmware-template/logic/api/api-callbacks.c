#include "api/api-commands.h"
#include "firmware.h"
#include "device/device.h"
#include "platform/platform.h"
#include "led-task/led-task.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

static void print_led_status(void)
{
    printf("led: %s, period %lu ms\r\n",
           led_state_name(led_task_get_state()),
           (unsigned long)led_task_get_period_ms());
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
    printf("uptime: %llu ms\r\n", (unsigned long long)(platform_time_us() / 1000ULL));
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
    if (command->argc != 1 || !parse_u32(command->argv, &period_ms))
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

static void cmd_calc_pi(const command_t *command)
{
    uint32_t terms = 1000000U;

    if (command->argc > 1)
    {
        printf("error: usage calc_pi [terms]\r\n");
        return;
    }

    if (command->argc == 1)
    {
        if (!parse_u32(command->argv, &terms))
        {
            printf("error: usage calc_pi [terms]\r\n");
            return;
        }
        if (terms == 0)
        {
            printf("error: terms must be greater than 0\r\n");
            return;
        }
    }

    uint64_t start_us = platform_time_us();
    double sum = 0.0;
    for (uint32_t k = 0; k < terms; k++)
    {
        double denom = 2.0 * (double)k + 1.0;
        if ((k & 1U) == 0U)
        {
            sum += 1.0 / denom;
        }
        else
        {
            sum -= 1.0 / denom;
        }
    }
    double pi = 4.0 * sum;
    uint64_t elapsed_ms = (platform_time_us() - start_us) / 1000ULL;

    printf("pi: %.9f\r\n", pi);
    printf("time: %llu ms\r\n", (unsigned long long)elapsed_ms);
}

const api_command_t g_api_commands[] = {
    { "info",        "device passport",               cmd_info },
    { "uptime",      "time since reset",              cmd_uptime },
    { "led_enable",  "turn led on",                   cmd_led_enable },
    { "led_disable", "turn led off",                  cmd_led_disable },
    { "led_blink",   "blink led",                     cmd_led_blink },
    { "led_period",  "set led blink period",          cmd_led_period },
    { "calc_pi",     "calculate pi: calc_pi [terms]", cmd_calc_pi },
};

const size_t g_api_commands_count = sizeof(g_api_commands) / sizeof(g_api_commands);

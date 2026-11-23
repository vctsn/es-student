#include "stdio-text-protocol/stdio-text-protocol.h"
#include "pico/stdlib.h"
#include <stdio.h>

static char s_line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint32_t s_pos = 0;
static bool s_overflow = false;
static command_t s_command;

void stdio_text_protocol_init(void)
{
    s_pos = 0;
    s_overflow = false;
}

static bool split_line(void)
{
    s_command.name = NULL;
    s_command.argc = 0;
    s_command.truncated = s_overflow;

    char *p = s_line;

    while (*p == ' ')
    {
        p++;
    }

    if (*p == '\0')
    {
        return false;
    }

    s_command.name = p;
    while (*p != '\0' && *p != ' ')
    {
        p++;
    }
    if (*p == ' ')
    {
        *p = '\0';
        p++;
    }

    while (*p != '\0')
    {
        while (*p == ' ')
        {
            p++;
        }
        if (*p == '\0')
        {
            break;
        }

        if (s_command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS)
        {
            s_command.argv[s_command.argc++] = p;
        }
        else
        {
            s_command.truncated = true;
        }

        while (*p != '\0' && *p != ' ')
        {
            p++;
        }
        if (*p == ' ')
        {
            *p = '\0';
            p++;
        }
    }

    return true;
}

const command_t *stdio_text_protocol_handle(void)
{
    int ch = getchar_timeout_us(0);
    if (ch == PICO_ERROR_TIMEOUT || ch < 0)
    {
        return NULL;
    }

    uint8_t c = (uint8_t)ch;

    if (c == '\r' || c == '\n')
    {
        if (s_pos == 0)
        {
            return NULL;
        }
        printf("\r\n");
        s_line[s_pos] = '\0';
        bool has_cmd = split_line();
        s_pos = 0;
        s_overflow = false;
        return has_cmd ? &s_command : NULL;
    }

    if (c == 0x08 || c == 0x7F)
    {
        if (s_pos > 0)
        {
            s_pos--;
            s_overflow = false;
            printf("\x08 \x08");
        }
        return NULL;
    }

    if (c >= 0x20 && c <= 0x7E)
    {
        if (s_pos < (STDIO_TEXT_PROTOCOL_LINE_SIZE - 1))
        {
            s_line[s_pos++] = (char)c;
            putchar((int)c);
        }
        else
        {
            s_overflow = true;
        }
        return NULL;
    }

    return NULL;
}

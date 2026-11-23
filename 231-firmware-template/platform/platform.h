#pragma once

#include <stdbool.h>
#include <stdint.h>

void platform_init(void);
uint64_t platform_time_us(void);
void platform_led_set(bool on);

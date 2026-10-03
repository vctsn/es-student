#pragma once

#include "pico/unique_id.h"
#include <stdio.h>

#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.0.0"

#define DEVICE_PROJECT "211-command-usb"
#define DEVICE_REPO "https://github.com/vctsn/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

void device_info();


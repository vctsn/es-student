#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PI_STATE_NOT_STARTED,
    PI_STATE_RUNNING,
    PI_STATE_DONE,
} pi_state_t;

void pi_task_init(void);
bool pi_task_start(uint32_t terms);
void pi_task_handle(void);
pi_state_t pi_task_get_state(void);
double pi_task_get_result(uint32_t *elapsed_ms);

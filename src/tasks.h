#ifndef TASKS_H
#define TASKS_H

#include "vehicle_state.h"

void* task_esc(void* arg); // 
void* task_tcs(void* arg);
void* task_injection(void* arg);
void* task_thermal(void* arg);

#endif /* TASKS_H */
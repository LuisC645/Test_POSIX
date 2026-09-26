#include "tasks.h"
#include "utils.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

#define PERIOD_INJ_NS (80L * 1000000L) // tau3: 80 ms = 12.5 Hz

// POSIX Clock.
// Cada 80 ms simula el RPM del motor y calcula el combustible

void* task_injection(void* arg) {

    VehicleState* vs = (VehicleState*)arg;

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    float t = 0.0f;

    while (1) {
        
        // Simular combustibe
        float rpm = 2000.0f + 500.0f * sinf(t * 0.5f);
        float fuel_ml = rpm * 0.0008f; // combustible proporcional al RPM

        pthread_mutex_lock(&vs->lock);
        vs->fuel_injected_ml += fuel_ml;
        float total = vs->fuel_injected_ml;
        pthread_mutex_unlock(&vs->lock);

        printf("[tau3-INJ] rpm=%.1f  fuel_ciclo=%.3f ml  fuel_total=%.3f ml\n",
               rpm, fuel_ml, total);

        // Esperar 80ms
        t += 0.08f;
        timespec_add_ns(&next, PERIOD_INJ_NS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
    return NULL;
}
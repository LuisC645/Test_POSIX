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

    for (int cycle = 0; cycle < CYCLES_INJ; cycle++) {

        // Obtener tiempo
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double current_abs = current_time.tv_sec + (current_time.tv_nsec / 1e9);

        // T_Real
        double real_timestamp = current_abs - start_timestamp;
        
        // Simular combustibe
        float fuel_ml = 1.6f + 0.4f * sinf(t * 0.5f);

        pthread_mutex_lock(&vs->lock);
        vs->fuel_injected_ml += fuel_ml;
        float total = vs->fuel_injected_ml;
        pthread_mutex_unlock(&vs->lock);

        double t_teorico = cycle * 0.080; // 80 ms = 0.080 segundos
        printf("[TAREA3 - Ciclo %3d] T_Real=%.6f s | T_Teorico=%.2f | fuel_ciclo=%.3f ml | fuel_total=%.3f ml\n",
                cycle, real_timestamp, t_teorico, fuel_ml, total);

        // Esperar 80ms
        t += 0.08f;
        timespec_add_ns(&next, PERIOD_INJ_NS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
    return NULL;
}
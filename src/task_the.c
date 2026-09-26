#include "tasks.h"
#include "utils.h"
#include <stdio.h>
#include <signal.h>

#define PERIOD_THERMAL_NS (200L * 1000000L) // tau4: 200 ms = 5 Hz
#define OFFSET_THERMAL_NS 0L
#define SIG_THERMAL (SIGRTMIN + 1) // senal RT propia de tau4 (distinta a SIG_TCS)

// POSIX timer
// Cada 200 ms simula la temperatura del motor y actualiza

void* task_thermal(void* arg) {
    VehicleState* vs = (VehicleState*)arg;

    struct periodic_task pt;
    start_periodic_timer(&pt, SIG_THERMAL, OFFSET_THERMAL_NS, PERIOD_THERMAL_NS);

    for (int cycle = 0; cycle < CYCLES_THEM; cycle++) {

        // Obtener tiempo
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double current_abs = current_time.tv_sec + (current_time.tv_nsec / 1e9);

        // T_Real
        double real_timestamp = current_abs - start_timestamp;

        wait_next_activation(&pt);

        // Job Body
        pthread_mutex_lock(&vs->lock);
        vs->engine_temp_c += 0.4f; // el motor calienta con el uso
        if (vs->engine_temp_c > 120.0f) {
            vs->engine_temp_c = 90.0f; // Se enfria si pasa 129
        }
        float temp = vs->engine_temp_c;
        pthread_mutex_unlock(&vs->lock);

        double t_teorico = cycle * 0.200;
        printf("[TAREA4 - Ciclo %3d] T_Real=%.6f s | T_Teorico=%.2f | temp_motor=%.2f C\n",
                cycle, real_timestamp, t_teorico, temp);
    }
    return NULL;
}
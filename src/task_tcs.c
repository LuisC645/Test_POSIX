#include "tasks.h"
#include "utils.h"
#include <stdio.h>
#include <signal.h>

#define PERIOD_TCS_NS (40L * 1000000L) // tau2: 40 ms = 25 Hz
#define OFFSET_TCS_NS 0L               // 0 offset inicual
#define SIG_TCS (SIGRTMIN + 0)         // senal RT dedicada a tau2

// POSIX timer
// Lee las 4 ruedas y calcula el deslizamiento

void* task_tcs(void* arg) {

    VehicleState* vs = (VehicleState*)arg;

    struct periodic_task pt;
    start_periodic_timer(&pt, SIG_TCS, OFFSET_TCS_NS, PERIOD_TCS_NS);

    for (int cycle = 0; cycle < CYCLES_TCS; cycle++) {

        // Obtener tiempo
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double current_abs = current_time.tv_sec + (current_time.tv_nsec / 1e9);

        // T_Real
        double real_timestamp = current_abs - start_timestamp;

        wait_next_activation(&pt);

        pthread_mutex_lock(&vs->lock);
        while (!vs->esc_data_ready) {
            pthread_cond_wait(&vs->esc_ready_cond, &vs->lock);
        }

        // Job Body: lee el recurso compartido wheel_speed[4]
        float fastest = vs->wheel_speed[0];
        float slowest = vs->wheel_speed[0];
        for (int i = 1; i < 4; i++) {
            if (vs->wheel_speed[i] > fastest) fastest = vs->wheel_speed[i];
            if (vs->wheel_speed[i] < slowest) slowest = vs->wheel_speed[i];
        }
        float slip_diff = fastest - slowest;

        vs->throttle_reduction_pct = (slip_diff > 1.0f) ? (slip_diff * 10.0f) : 0.0f;
        vs->esc_data_ready = 0; // dato usado

        pthread_mutex_unlock(&vs->lock);

        double t_teorico = cycle * 0.040; // 40 ms = 0.040 segundos
        printf("[TAREA2 - Ciclo %3d] T_Real=%.6f s | T_Teorico=%.2f | slip_diff=%.2f, throttle=%.1f%%\n",
                cycle, real_timestamp, t_teorico, slip_diff, vs->throttle_reduction_pct);
    }
    return NULL;
}
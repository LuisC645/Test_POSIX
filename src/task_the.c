#include "tasks.h"
#include "utils.h"
#include <stdio.h>
#include <signal.h>

#define PERIOD_THERMAL_NS (200L * 1000000L) // tau4: 200 ms = 5 Hz
#define OFFSET_THERMAL_NS 0L
#define SIG_THERMAL (SIGRTMIN + 1) // senal RT propia de tau4 (distinta a SIG_TCS)
#define TEMP_WARNING_C 105.0f

// POSIX timer
// Cada 200 ms simula la temperatura del motor y actualiza

void* task_thermal(void* arg) {
    VehicleState* vs = (VehicleState*)arg;

    struct periodic_task pt;
    start_periodic_timer(&pt, SIG_THERMAL, OFFSET_THERMAL_NS, PERIOD_THERMAL_NS);

    while (1) {

        wait_next_activation(&pt);

        // Job Body
        pthread_mutex_lock(&vs->lock);
        vs->engine_temp_c += 0.4f; // el motor calienta con el uso
        if (vs->engine_temp_c > 120.0f) {
            vs->engine_temp_c = 90.0f; // Se enfria si pasa 129
        }
        float temp = vs->engine_temp_c;
        pthread_mutex_unlock(&vs->lock);

        printf("[tau4-TEMP] temp_motor=%.2f C%s\n", temp,
               (temp > TEMP_WARNING_C) ? "  *** ALERTA DE SOBRECALENTAMIENTO ***" : "");
    }
    return NULL;
}
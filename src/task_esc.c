#include <stdio.h>
#include <math.h>

#include "tasks.h"
#include "time.h"

#define PERIOD_ESC_NS 20000000L // tau1: 20 ms = 50 hz

// POSIX Clock
// Simula velocidad de las 4 ruedas
// Usa el arreglo wheel_speed[4] para guardar velocidades

void* task_esc(void* arg) {

    VehicleState* vs = (VehicleState*)arg;
    
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    float t = 0.0;

    while (1) {
        
        // Simulacion de sensores
        float vehicle_speed = 60.0 + 5.0 * sin(t);
        
        float wheel[4];
        wheel[0] = vehicle_speed * (1.0 - 0.02 * fabs(sin(t * 3.0)));
        wheel[1] = vehicle_speed * (1.0 - 0.02 * fabs(sin(t * 3.0 + 1.0)));
        wheel[2] = vehicle_speed * (1.0 - 0.02 * fabs(sin(t * 3.0 + 2.0)));
        wheel[3] = vehicle_speed * (1.0 - 0.02 * fabs(sin(t * 3.0 + 3.0)));

        pthread_mutex_lock(&vs->lock);
        vs->speed_kmh = vehicle_speed;
        for (int i = 0; i < 4; i++) {
            vs->wheel_speed[i] = wheel[i];
        }
        vs->esc_data_ready = 1;
        pthread_cond_signal(&vs->esc_ready_cond);
        pthread_mutex_unlock(&vs->lock);

        printf("TAREA1: t=%.2f  v=%.2f km/h  ruedas=[%.2f %.2f %.2f %.2f]\n",
               t, vehicle_speed, wheel[0], wheel[1], wheel[2], wheel[3]);

        t += 0.02;
        timespec_add_ns(&next, PERIOD_ESC_NS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

    }
    return NULL;
}
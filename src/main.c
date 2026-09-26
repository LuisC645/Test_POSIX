#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include "vehicle_state.h"
#include "tasks.h"

#define SIG_TCS     (SIGRTMIN + 0) // tcs
#define SIG_THERMAL (SIGRTMIN + 1) // thermal

int main(void) {

    VehicleState vs;
    vehicle_state_init(&vs);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIG_TCS);
    sigaddset(&set, SIG_THERMAL);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    pthread_t th_esc, th_tcs, th_inj, th_thermal;

    pthread_create(&th_esc, NULL, task_esc, &vs); // POSIX Clock
    pthread_create(&th_tcs, NULL, task_tcs, &vs); // POSIX Timer
    pthread_create(&th_inj, NULL, task_injection, &vs); // POSIX Clock
    pthread_create(&th_thermal, NULL, task_thermal,   &vs); // POSIX Timer

    pthread_join(th_esc, NULL);
    pthread_join(th_tcs, NULL);
    pthread_join(th_inj, NULL);
    pthread_join(th_thermal, NULL);

    vehicle_state_destroy(&vs);
    return 0;
}
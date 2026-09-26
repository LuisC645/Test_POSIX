#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <signal.h>   // sigset_t, sigemptyset, sigaddset, sigwait
#include <string.h>   // memset
#include <pthread.h>  // pthread_sigmask
#include <stdio.h>    // printf

// Ciclos
#define SIM_TIME_SEC 5 // 5s Sim

#define CYCLES_ESC (SIM_TIME_SEC * 1000 / 20)  // tau1 (500 ciclos)
#define CYCLES_TCS (SIM_TIME_SEC * 1000 / 40)  // tau2 (250 ciclos)
#define CYCLES_INJ (SIM_TIME_SEC * 1000 / 80)  // tau3 (125 ciclos)
#define CYCLES_THEM (SIM_TIME_SEC * 1000 / 200) // tau4 (50 ciclos)

extern double start_timestamp; // Almacenar tiempo de inicio

// CLocks

#define NSEC_PER_SEC 1000000000L

// https://github.com/gpatigno/RTS_2026-2/blob/main/MODULO2/POSIX-RT/TIMERS_CLOCKS/Example12/periodic_settings.c
// Recibe ns a diferencia de la referencia que recibe us (mas directa)

static inline void timespec_add_ns(struct timespec* t, long ns) {
    t->tv_nsec += ns;
    while (t->tv_nsec >= NSEC_PER_SEC) {
        t->tv_nsec -= NSEC_PER_SEC;
        t->tv_sec++;
    }
}

// Timers
// https://github.com/gpatigno/RTS_2026-2/blob/main/MODULO2/POSIX-RT/TIMERS_CLOCKS/example9_periodic-task.c
// 

struct periodic_task {
    sigset_t sigset;
    struct itimerspec t;
    struct sigevent sigev;
    timer_t timer;
    long period; // long por ns
};

static inline void start_periodic_timer(struct periodic_task* p, 
                                        int signum, 
                                        long offset_ns, 
                                        long period_ns
                                        ) {

    memset(&p->t, 0, sizeof(p->t));
    memset(&p->sigev, 0, sizeof(p->sigev));

    sigemptyset(&p->sigset);
    sigaddset(&p->sigset, signum);
    pthread_sigmask(SIG_BLOCK, &p->sigset, NULL);

    p->sigev.sigev_notify = SIGEV_SIGNAL;
    p->sigev.sigev_signo  = signum;

    timer_create(CLOCK_MONOTONIC, &p->sigev, &p->timer); // evita derivas de reloj

    // No tener offset 0 = se crashea
    if (offset_ns == 0) {
        offset_ns = period_ns;
    }

    p->t.it_value.tv_sec     = offset_ns / NSEC_PER_SEC;
    p->t.it_value.tv_nsec    = offset_ns % NSEC_PER_SEC;
    p->t.it_interval.tv_sec  = period_ns / NSEC_PER_SEC;
    p->t.it_interval.tv_nsec = period_ns % NSEC_PER_SEC;

    p->period = period_ns;
    timer_settime(p->timer, 0, &p->t, NULL);
}

// Bloquea el hilo hasta que el timer dispare su senal asociada.
static inline void wait_next_activation(struct periodic_task* p) {
    int sig;
    sigwait(&p->sigset, &sig);
}

#endif // UTILS_H
#ifndef VEHICLE_STATE_H
#define VEHICLE_STATE_H

#include <pthread.h>
#include <stdint.h>

typedef struct {

    // Compartidos
    float wheel_speed[4]; // Velocidad de cada rueda

    // tau1 (ESP/ESP)
    float speed_kmh;

    // tau2 (TCS)
    float throttle_reduction_pct;

    // tau3 (Inyeccion)
    float fuel_injected_ml;

    // tau4 (Monitor termico)
    float engine_temp_c;

    // Bandera para la variable condicional
    int esc_data_ready; 

    // Sincronizacion
    pthread_mutex_t lock; 
    pthread_cond_t esc_ready_cond; // Verificar bloqueo/derrape

} VehicleState;

void vehicle_state_init(VehicleState *vs);
void vehicle_state_destroy(VehicleState *vs);

#endif /* VEHICLE_STATE_H */
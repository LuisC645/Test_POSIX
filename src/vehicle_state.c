#include "vehicle_state.h"

void vehicle_state_init(VehicleState *vs) {
    
    // Compartidos
    for(unsigned short i = 0; i < 4; i++) {
        vs->wheel_speed[i] = 0.0;
    }

    // tau1 (ESP/ESP)
    vs->speed_kmh = 0.0;
    
    // tau2 (TCS)
    vs->throttle_reduction_pct = 0.0;

    // tau3 (Inyeccion)
    vs->fuel_injected_ml = 0.0;

    // tau4 (Monitor termico)
    vs->engine_temp_c = 90.0;

    // Bandera
    vs->esc_data_ready = 0;

    // Sincronizacion
    // Inicializar sincronizacion (mutex y variable condicional)
    pthread_mutex_init(&vs->lock, NULL);
    pthread_cond_init(&vs->esc_ready_cond, NULL);

}

void vehicle_state_destroy(VehicleState *vs) {

    // Liberar recursos de sincronizacion
    pthread_mutex_destroy(&vs->lock);
    pthread_cond_destroy(&vs->esc_ready_cond);

}

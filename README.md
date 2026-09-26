# Sistema de Control de Automóvil en Tiempo Real (POSIX-RT)

Sistema de control automotriz simulado, implementado con **hilos periódicos POSIX-RT** en C. El sistema modela 4 subsistemas de un automóvil (control de estabilidad, control de tracción, inyección de combustible y monitoreo térmico), cada uno ejecutándose como una tarea periódica independiente, con sincronización entre tareas mediante mutex y variables condicionales.

---

## Descripción general

El sistema simula 4 tareas de tiempo real que se ejecutan de forma concurrente, cada una con su propio período y su propio mecanismo de temporización POSIX:

| Tarea | Subsistema | Período | Mecanismo POSIX |
|---|---|---|---|
| **τ1 — ESC/ESP** | Control de Estabilidad | 20 ms (50 Hz) | POSIX Clock (`clock_nanosleep`) |
| **τ2 — TCS** | Control de Tracción | 40 ms (25 Hz) | POSIX Timer (`timer_create` + señal RT) |
| **τ3 — INJ** | Inyección de Combustible | 80 ms (12.5 Hz) | POSIX Clock (`clock_nanosleep`) |
| **τ4 — TEMP** | Monitor Térmico | 200 ms (5 Hz) | POSIX Timer (`timer_create` + `SIGEV_THREAD`) |

τ1 y τ2 comparten un recurso de datos (`wheel_speed[4]`, la velocidad de las 4 ruedas) protegido por un **mutex** y sincronizado con una **variable condicional**: τ1 publica los datos y τ2 los consume para calcular el deslizamiento de las ruedas.

---

## Tecnologías y mecanismos usados

- **C (ANSI C / C99)**
- **POSIX Threads** (`pthread`) — hilos, mutex, variables condicionales
- **POSIX Timers** (`timer_create`, `timer_settime`) — con notificación por señal RT (`SIGEV_SIGNAL`) y por hilo callback (`SIGEV_THREAD`)
- **POSIX Clocks** (`clock_gettime`, `clock_nanosleep`) — temporización manual con tiempo absoluto, sin *drift*
- **Real-Time Signals** (`SIGRTMIN+n`)
- **Cross-compilación** para ARM64 (`aarch64-linux-gnu-gcc`) y despliegue remoto por SSH/SCP a una Raspberry Pi 4

---

## Estructura del repositorio

```
LAB3_RTS/
├── src/
│   ├── main.c              # Punto de entrada: crea el recurso compartido y lanza los 4 hilos
│   ├── vehicle_state.h     # Struct VehicleState (recurso compartido) + prototipos
│   ├── vehicle_state.c     # Init/destroy del recurso compartido (mutex + cond var)
│   ├── tasks.h             # Prototipos de las 4 tareas
│   ├── utils.h             # Helpers reutilizables para Clocks y Timers POSIX
│   ├── task_esc.c          # tau1 - Control de Estabilidad   (Clock,  20 ms)
│   ├── task_tcs.c          # tau2 - Control de Traccion      (Timer,  40 ms)
│   ├── task_inj.c          # tau3 - Inyeccion de Combustible (Clock,  80 ms)
│   └── task_the.c          # tau4 - Monitor Termico          (Timer, 200 ms)
├── bin/                     # Ejecutables generados (no versionado)
├── results/                 # Logs de ejecucion (no versionado)
└── Makefile
```

---

## Requisitos

**Para compilar y correr en PC (Linux / WSL):**
- `gcc`
- `make`

**Para cross-compilar y desplegar en la Raspberry Pi 4 (arquitectura de 64 bits):**
- `gcc-aarch64-linux-gnu` (`sudo apt install gcc-aarch64-linux-gnu`)
- Acceso por SSH a la Raspberry Pi 4 configurado (recomendado: llave SSH sin contraseña con `ssh-copy-id`)

---

## Compilación y ejecución

### En PC (nativo)

```bash
make              # compila -> bin/vehicle_rts
make run          # compila y ejecuta, mostrando la salida en consola
make log-pc       # compila, ejecuta y guarda la salida en results/log_pc.txt
```

### Cross-compilación y ejecución en la Raspberry Pi 4

```bash
make cross        # cross-compila -> bin/vehicle_rts_pi
make deploy-rpi   # crea la carpeta remota y copia el binario a la RPi4 por SCP
make run-rpi      # ejecuta el binario en la RPi4 via SSH, mostrando la salida en tu terminal local
make log-rpi      # ejecuta en la RPi4 via SSH y guarda la salida localmente en results/log_rpi.txt
```

> La IP y el usuario de la Raspberry Pi 4 se configuran al inicio del `Makefile` (`RPI_USER`, `RPI_HOST`, `RPI_DIR`).

### Otros targets útiles

```bash
make debug        # compila con simbolos de depuracion (-g -O0), util con gdb/valgrind
make rebuild       # limpia y recompila desde cero
make clean         # borra objetos y ejecutables
make help          # lista todos los targets disponibles
```

Para detener la ejecución (las 4 tareas corren en un ciclo infinito, como un sistema de control real): **Ctrl+C**.

---

## Resultados: determinismo y periodicidad

Se midió, para cada tarea, el período real de ejecución (`clock_gettime` en cada ciclo) contra el período teórico de la Tabla 1, tanto en PC como en la Raspberry Pi 4:

| Tarea | Período teórico | Promedio PC | Jitter PC | Promedio RPi4 | Jitter RPi4 |
|---|---|---|---|---|---|
| τ1-ESC | 20 ms | 20.001 ms | 0.983 ms | 20.000 ms | 0.886 ms |
| τ2-TCS | 40 ms | 40.002 ms | 0.740 ms | 40.001 ms | 0.637 ms |
| τ3-INJ | 80 ms | 80.005 ms | 0.916 ms | 80.002 ms | 0.256 ms |
| τ4-TEMP | 200 ms | 200.014 ms | 0.426 ms | 200.009 ms | 2.460 ms |

**Hallazgos principales:**
- **No se observó drift** en ninguna tarea ni plataforma: el período promedio real coincide con el teórico hasta el tercer/cuarto decimal, confirmando que el uso de tiempo absoluto (`TIMER_ABSTIME` y timers con reencolado de señales RT) evita la acumulación de error a lo largo del tiempo.
- τ1, τ2 y τ3 mostraron **menor jitter en la Raspberry Pi 4** que en el PC (Linux nativo dedicado vs. WSL compartiendo recursos con Windows).
- τ4 mostró **mayor jitter en la Raspberry Pi 4** (2.46 ms vs. 0.43 ms en PC) — atribuible a que usa `SIGEV_THREAD`, que crea un hilo nuevo en cada disparo del timer, un costo que un procesador más limitado como el de la RPi4 absorbe con mayor variabilidad.

El uso de los 4 núcleos de la Raspberry Pi 4 durante la ejecución fue verificado con `htop`.

---

## Recurso compartido y sincronización

El `struct VehicleState` (definido en `vehicle_state.h`) es el recurso compartido entre tareas, protegido por:

- **`pthread_mutex_t lock`** — exclusión mutua en todo acceso al struct.
- **`pthread_cond_t esc_ready_cond`** — sincronización entre τ1 (productor) y τ2 (consumidor): τ1 publica `wheel_speed[4]` y notifica con `pthread_cond_signal`; τ2 espera con `pthread_cond_wait` hasta que hay un dato nuevo, evitando lecturas repetidas o inconsistentes.

---

## Autor

Luis Castillo Chicaiza — Ingeniería Electrónica, Universidad de Antioquia.
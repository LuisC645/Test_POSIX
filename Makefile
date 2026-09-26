CC      = gcc
CFLAGS  = -Wall -Wextra -Isrc -pthread
LDFLAGS = -lpthread -lrt -lm

SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:.c=.o)
BIN_DIR = bin

TARGET      = $(BIN_DIR)/vehicle_rts     # ejecutable para PC (nativo)
TARGET_PI   = $(BIN_DIR)/vehicle_rts_pi  # ejecutable cross-compilado para RPi4

RESULTS_DIR = results
LOG_PC      = $(RESULTS_DIR)/log_pc.txt
LOG_RPI     = $(RESULTS_DIR)/log_rpi.txt

# --- Conexion a la Raspberry Pi 4 ---
RPI_USER = pi
RPI_HOST = 192.168.1.178
RPI_DIR  = Documents/lab3

# ------------------------------------------------------------
#  Compilacion normal en PC (por defecto al escribir solo "make")
# ------------------------------------------------------------

all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(RESULTS_DIR):
	mkdir -p $(RESULTS_DIR)

rebuild: clean all

clean:
	rm -f $(OBJ)
	rm -rf $(BIN_DIR)

# ------------------------------------------------------------
#  Ejecucion en PC
# ------------------------------------------------------------

run: all
	./$(TARGET)

log-pc: all | $(RESULTS_DIR)
	./$(TARGET) > $(LOG_PC)

# ------------------------------------------------------------
#  Compilacion con simbolos de depuracion
# ------------------------------------------------------------
debug: CFLAGS += -g -O0
debug: clean all

# ------------------------------------------------------------
#  Cross-compilacion para Raspberry Pi 4 (aarch64, 64 bits)
# ------------------------------------------------------------
CROSS_CC     = aarch64-linux-gnu-gcc
CROSS_CFLAGS = -Wall -Wextra -pedantic -ggdb3 -O0 -Isrc -pthread

cross: $(SRC) | $(BIN_DIR)
	$(CROSS_CC) $(CROSS_CFLAGS) -o $(TARGET_PI) $(SRC) $(LDFLAGS)

# Crea la carpeta remota (Documents/lab3/bin) y copia el binario ahi.
# No asume que exista nada previo en la RPi -- la crea desde cero.
deploy-rpi: cross
	ssh $(RPI_USER)@$(RPI_HOST) 'mkdir -p $(RPI_DIR)/bin $(RPI_DIR)/results'
	scp $(TARGET_PI) $(RPI_USER)@$(RPI_HOST):$(RPI_DIR)/bin/vehicle_rts_pi

# ------------------------------------------------------------
#  Ejecucion en la Raspberry Pi 4 (via SSH)
# ------------------------------------------------------------

run-rpi:
	ssh -tt $(RPI_USER)@$(RPI_HOST) 'cd $(RPI_DIR) && ./bin/vehicle_rts_pi'

log-rpi: | $(RESULTS_DIR)
	ssh -tt $(RPI_USER)@$(RPI_HOST) 'cd $(RPI_DIR) && ./bin/vehicle_rts_pi' > $(LOG_RPI)

# ------------------------------------------------------------
#  Ayuda
# ------------------------------------------------------------
help:
	@echo "Targets disponibles:"
	@echo "  make            - compila el proyecto para PC (bin/vehicle_rts)"
	@echo "  make run        - compila y ejecuta en PC (sin guardar log)"
	@echo "  make log-pc     - compila, ejecuta en PC y guarda results/log_pc.txt"
	@echo "  make cross      - cross-compila para RPi4 (bin/vehicle_rts_pi)"
	@echo "  make deploy-rpi - crea Documents/lab3 en la RPi4 y copia el binario"
	@echo "  make run-rpi    - ejecuta vehicle_rts_pi en la RPi4 via SSH (sin guardar log)"
	@echo "  make log-rpi    - ejecuta vehicle_rts_pi en la RPi4 via SSH y guarda results/log_rpi.txt"
	@echo "  make rebuild    - limpia y recompila desde cero"
	@echo "  make debug      - compila con simbolos para gdb/valgrind"
	@echo "  make clean      - borra objetos y ejecutables"

.PHONY: all run log-pc rebuild clean debug cross deploy-rpi run-rpi log-rpi help
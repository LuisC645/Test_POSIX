CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude -pthread
LDFLAGS = -lpthread -lrt -lm

SRC    = $(wildcard src/*.c)
OBJ    = $(SRC:.c=.o)
TARGET = vehicle_rts

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

OUT = chip8
CC = gcc

CFLAGS := $(shell sdl-config --cflags)
LDFLAGS := $(shell sdl-config --libs)

OBJECTS = main.o ../core/chip8.o

all: $(OUT)
$(OUT): $(OBJECTS)
	$(CC) -o $@ $^ ${CFLAGS} ${LDFLAGS}

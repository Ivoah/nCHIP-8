OUT = chip8
CXX = g++

CFLAGS := $(shell sdl-config --cflags)
LDFLAGS := $(shell sdl-config --libs)

OBJECTS = main.o chip8.o

all: $(OUT)
$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^ ${CFLAGS} ${LDFLAGS}

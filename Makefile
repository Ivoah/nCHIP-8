OUT = chip8
CXX = g++

SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)

OBJECTS = main.o chip8.o

all: $(OUT)
$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^ ${SDL_CFLAGS} ${SDL_LDFLAGS}

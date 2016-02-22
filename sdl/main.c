#include <SDL/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "../core/chip8.h"

#define SCALE			7

#define SCREEN_WIDTH	64*SCALE
#define SCREEN_HEIGHT	32*SCALE
#define SCREEN_BPP		32

int chip_to_sdl(uint8_t key);
int sdl_to_chip(uint8_t key);
void setPixel(uint32_t* pixels, int x, int y, int size, uint32_t color);

int main(int argc, char* argv[]) {

	SDL_Surface* screen = NULL;
	SDL_Event event;
	bool quit = false;
	uint8_t keys[16];

	struct chip8 emu;
	chip8_init(&emu, false);

	if (argc == 2) {
		chip8_loadROM(&emu, argv[1]);
	}
	SDL_Init(SDL_INIT_EVERYTHING);

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	uint32_t* pixels = (uint32_t*)screen->pixels;

	while (!quit) {

		for (int i = 0; i < 50; i++) {
			chip8_step(&emu);
		}

		if (emu.updateScreen) {
			SDL_LockSurface(screen);
			for (int y = 0; y < 32; y++) {
				//uint32_t *line = (uint32_t*)((char *)screen->pixels + y * screen->pitch);
				for (int x = 0; x < 64; x++) {
					if (emu.gfx[(y*64) + x]) {
						setPixel(pixels, x*SCALE, y*SCALE, SCALE, 0xFFFFFFFF);
					} else {
						setPixel(pixels, x*SCALE, y*SCALE, SCALE, 0x00000000);
					}
				}
			}
			SDL_UnlockSurface(screen);
			emu.updateScreen = false;
		}

		SDL_Flip(screen);

		uint8_t *sdl_keys = SDL_GetKeyState(NULL);

		for (int i = 0; i < 16; i++) {
			emu.keys[i] = sdl_keys[chip_to_sdl(i)];
		}

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				emu.wait_key = sdl_to_chip(event.key.keysym.sym);
				fprintf(stderr, "Pressed key %x", emu.wait_key);
				break;
			}
		}
	}

	SDL_Quit();

	return 0;
}

void setPixel(uint32_t* pixels, int x, int y, int size, uint32_t color) {
	for (int _y = 0; _y < size; _y++) {
		for (int _x = 0; _x < size; _x++) {
			pixels[((y+_y)*SCREEN_WIDTH) + x+_x] = color;
		}
	}
}

enum {
	Key_0 = 0, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9, Key_A, Key_B, Key_C, Key_D, Key_E, Key_F
};

int chip_to_sdl(uint8_t key) {
        switch (key) {
	        case Key_1: return SDLK_1;
	        case Key_2: return SDLK_2;
	        case Key_3: return SDLK_3;
	        case Key_C: return SDLK_4;
	        case Key_4: return SDLK_q;
	        case Key_5: return SDLK_w;
	        case Key_6: return SDLK_e;
	        case Key_D: return SDLK_r;
	        case Key_7: return SDLK_a;
	        case Key_8: return SDLK_s;
	        case Key_9: return SDLK_d;
	        case Key_E: return SDLK_f;
	        case Key_A: return SDLK_z;
	        case Key_0: return SDLK_x;
	        case Key_B: return SDLK_c;
	        case Key_F: return SDLK_v;
        }
        return -1;
}

int sdl_to_chip(uint8_t key) {
        switch (key) {
			case SDLK_1: return Key_1;
			case SDLK_2: return Key_2;
			case SDLK_3: return Key_3;
			case SDLK_4: return Key_C;
			case SDLK_q: return Key_4;
			case SDLK_w: return Key_5;
			case SDLK_e: return Key_6;
			case SDLK_r: return Key_D;
			case SDLK_a: return Key_7;
			case SDLK_s: return Key_8;
			case SDLK_d: return Key_9;
			case SDLK_f: return Key_E;
			case SDLK_z: return Key_A;
			case SDLK_x: return Key_0;
			case SDLK_c: return Key_B;
			case SDLK_v: return Key_F;

        }
        return -1;
}

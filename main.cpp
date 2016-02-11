#include <SDL/SDL.h>
#include <cstdint>
#include <cstdio>

#include "chip8.h"

#define SCREEN_WIDTH	64
#define SCREEN_HEIGHT	32
#define SCREEN_BPP		32

int translate_key(uint8_t key);

int main(int argc, char* argv[]) {

	SDL_Surface* screen = NULL;
	SDL_Event event;
	bool quit = false;
	uint8_t keys[16];

	chip8 emu(true);

	if (argc == 2) {
		emu.loadROM(argv[1]);
	}
	SDL_Init(SDL_INIT_EVERYTHING);

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	uint32_t* pixels = (uint32_t*)screen->pixels;

	/*
	SDL_PixelFormat* fmt = screen->format;
	SDL_LockSurface(screen);
		for (int y = 0; y < 32; y++) {
			for (int x = 0; x < 64; x++) {
				pixels[(y*32) + x] = 0xFFFFFFFF;
			}
		}
	SDL_UnlockSurface(screen);
	SDL_Flip(screen);
	*/

	while (!quit) {

		emu.step();

		if (emu.updateScreen) {
			SDL_LockSurface(screen);
			for (int y = 0; y < 32; y++) {
				//uint32_t *line = (uint32_t*)((char *)screen->pixels + y * screen->pitch);
				for (int x = 0; x < 64; x++) {
					if (emu.gfx[(y*64) + x])
						pixels[(y*64) + x] = 0xFFFFFFFF;
					else
						pixels[(y*64) + x] = 0x00000000;
				}
			}
			SDL_UnlockSurface(screen);
			emu.updateScreen = false;
		}

		SDL_Flip(screen);

		uint8_t *sdl_keys = SDL_GetKeyState(NULL);

		for (int i = 0; i < 16; i++)
			emu.keys[i] = sdl_keys[translate_key(i)];

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			}
		}
	}

	SDL_Quit();

	return 0;
}

enum {
	Key_0, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9, Key_A, Key_B, Key_C, Key_D, Key_E, Key_F
};


int translate_key(uint8_t key) {
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

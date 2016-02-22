#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

//#define debug_print(fmt, ...) \
            if (debug) fprintf(stderr, fmt, ##__VA_ARGS__)

#define debug_print(...) \
            if (emu->debug) fprintf(stderr, __VA_ARGS__)


struct chip8 {
	//RAM
	uint8_t mem[4096];

	//Registers
	uint8_t V[16];
	uint16_t I;
	uint16_t pc;

	//Timers
	uint8_t delay_timer;
	uint8_t sound_timer;

	//Stack
	uint16_t stack[16];
	uint16_t sp;

	//Pubilc vars
	bool updateScreen;
	bool debug;
	uint8_t gfx[64 * 32];
	uint8_t keys[16];
    int wait_key;
};

void chip8_init(struct chip8* emu, bool dbg);
bool chip8_step(struct chip8* emu);
void chip8_loadROM(struct chip8* emu, char* ROM_NAME);
#endif

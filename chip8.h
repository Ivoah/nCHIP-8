#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#define debug_print(fmt, ...) \
            if (debug) fprintf(stderr, fmt, ##__VA_ARGS__)

class chip8 {
private:
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

	//Font
	static uint8_t font[80];

public:
	//Pubilc vars
	bool updateScreen;
	bool debug;
	uint8_t gfx[64 * 32];
	uint8_t keys[16];

	//Public functions
	chip8(bool);
	bool step();
	void loadROM(char*);
};

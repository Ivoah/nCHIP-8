#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>

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
	uint8_t gfx[64 * 32];
	uint8_t keys[16];

	//Public functions
	void init();
	bool step();
	void loadROM(char*);
};
#include "chip8.h"

void chip8::init() {

	//Seed rng
	srand(time(NULL));

	//Initilise pointers
	pc = 0x200;
	sp = 0;
	I = 0;

	//Initilise gfx
	updateScreen = true;
	for (int i = 0; i < 64*32; i++) {
		gfx[i] = 0;
	}

	//Initilise memory
	for (int i = 0; i < 4096; i++) {
		mem[0] = 0x0;
	}

	//Initilise registers
	for (int i = 0x0; i < 0xF; i++) {
		V[i] = 0x0;
	}

	//Load font
	for (int i = 0x0; i < 0x50; i++) {
		mem[i] = font[i];
	}
}

bool chip8::step() {

	//Decrement timers
	delay_timer--;
	sound_timer--;

	//Fetch opcode
	uint16_t opcode = mem[pc] << 8 | mem[pc + 1];

	//Decode opcode
	printf("Opcode: 0x%x at address 0x%x, 0x%x in file\n", opcode, pc, pc - 0x200);
	switch(opcode & 0xF000){
		case 0x0000:
			switch(opcode & 0x00FF) {
				case 0x00E0: //Clears the screen
					printf("Clearing screen\n");
					for (int y = 0; y < 32; y++)
						for (int x = 0; x < 64; x++)
							gfx[x + (y * 64)] = 0;
					pc += 2;
					break;
				case 0x00EE: //Returns from a subroutine
					printf("pc before return: 0x%x\n",pc);
					pc = stack[sp];
					printf("pc after return: 0x%x\n",pc);
					sp--;
					break;
			}
			break;
		case 0x1000: //Jumps to address NNN
			pc = opcode & 0x0FFF;
			break;
		case 0x2000: //Calls subroutine at NNN
			stack[sp] = pc;
			sp++;
			pc = opcode & 0x0FFF;
			break;
		case 0x3000:
			if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
			break;
		case 0x4000:
			if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
			break;
		case 0x5000:
			if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
			break;
		case 0x6000:
			V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			pc += 2;
			break;
		case 0x7000:
			V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			pc += 2;
			break;
		case 0x8000:
			switch(opcode & 0x000F) {
				case 0x0000:
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0001:
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0002:
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0003:
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0004:
					if (V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4] > 0xFF)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];;
					pc += 2;
					break;
				case 0x0005:
					if (V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4] < 0)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0006:
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					pc += 2;
					break;
				case 0x0007:
					if (V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8] < 0)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				case 0x000E:
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x8000;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					pc += 2;
					break;
			}
			break;
		case 0x9000:
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc +=2;
			break;
		case 0xA000:
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		case 0xB000:
			pc = (opcode & 0x0FFF) + V[0];
			break;
		case 0xC000:
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
			break;
		case 0xD000: {
			uint8_t x = V[(opcode & 0x0F00) >> 8];
			uint8_t y = V[(opcode & 0x00F0) >> 4];
			uint8_t h = opcode & 0x000F;
			uint8_t pxl;
			printf("XOR at (%d, %d)\n",x,y);
			for (int _y = 0; _y < h; _y++) {
				pxl = mem[I + _y];
				for (int _x = 0; _x < 8; _x++) {
					if ((pxl & (0x80 >> _x)) != 0) {
						if (gfx[(x + _x + ((y + _y) * 64))] == 1)
							V[0xF] = 1;
						gfx[(x + _x + ((y + _y) * 64))] ^= 1;
					}
				}
			}

			updateScreen = true;
			pc += 2;
			break;
		}
		case 0xE000:
			switch(opcode & 0x00FF) {
				case 0x009E:
					if (keys[V[(opcode & 0x0F00) >> 8]])
						pc += 4;
					else
						pc += 2;
					break;
				case 0x00A1:
					if (!keys[V[(opcode & 0x0F00) >> 8]])
						pc += 4;
					else
						pc += 2;
					break;
			}
			break;
		case 0xF000:
			switch (opcode & 0x00FF) {
				case 0x0007:
					V[(opcode & 0x0F00) >> 8] = delay_timer;
					pc += 2;
					break;
				case 0x000A:
					printf("Unimplemented\n");
					pc += 2;
					break;
				case 0x0015:
					delay_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				case 0x0018:
					sound_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				case 0x001E:
					I += V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				case 0x0029:
					I = V[(opcode & 0x0F00) >> 8] * 5;
					pc += 2;
					break;
				case 0x0033:
					mem[I] = ((opcode & 0x0F00) >> 8) / 100;
					mem[I + 1] = (((opcode & 0x0F00) >> 8) / 10) % 10;
					mem[I + 2] = (((opcode & 0x0F00) >> 8) % 100) % 10;
					pc += 2;
					break;
				case 0x0055:
					for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
						mem[I + i] = V[i];
					pc += 2;
					break;
				case 0x0065:
					for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
						V[i] = mem[I + i];
					pc += 2;
					break;
			}
			break;
		default:
			return false;
	}

	return true;
}

void chip8::loadROM(char* ROM_NAME) {
	FILE* rom = fopen(ROM_NAME, "r");
	fread(&mem[0x200], 8, 0xFFF - 0x200, rom);
	fclose(rom);
	printf("Loaded ROM: %s\n", ROM_NAME);
}

uint8_t chip8::font[80] = { 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


#include "chip8.h"

uint8_t chip8_font[];

void chip8_init(struct chip8* emu, bool dbg) {

	//Seed rng
	srand(time(NULL));

	//Initilise pointers
	emu->pc = 0x200;
	emu->sp = 0;
	emu->I = 0;

	emu->debug = dbg;

	//Initialize keys
	emu->wait_key = -1;
	for (int i = 0; i < 0xF; i++) {
		emu->keys[i] = 0;
	}

	//Initialize gfx
	emu->updateScreen = true;
	for (int i = 0; i < 64*32; i++) {
		emu->gfx[i] = 0;
	}

	//Initialize memory
	for (int i = 0; i < 4096; i++) {
		emu->mem[0] = 0x0;
	}

	//Initialize registers
	for (int i = 0x0; i < 0xF; i++) {
		emu->V[i] = 0x0;
	}

	//Load font
	for (int i = 0x0; i < 0x50; i++) {
		emu->mem[i] = chip8_font[i];
	}
}

bool chip8_step(struct chip8* emu) {

	//Decrement timers
	emu->delay_timer--; if (emu->delay_timer < 0) emu->delay_timer = 0;
	emu->sound_timer--; if (emu->sound_timer < 0) emu->sound_timer = 0;

	//Fetch opcode
	uint16_t opcode = emu->mem[emu->pc] << 8 | emu->mem[emu->pc + 1];

	//Decode opcode
	debug_print("Opcode: 0x%x at address 0x%x, 0x%x in file\n", opcode, emu->pc, emu->pc - 0x200);
	switch(opcode & 0xF000){
		case 0x0000:
			switch(opcode & 0x00FF) {
				case 0x00E0: //00E0: Clears the screen.
					debug_print("Clearing screen\n");
					memset(emu->gfx, 0, 64*32);
					emu->pc += 2;
					break;
				case 0x00EE: //00EE: Returns from a subroutine.
					debug_print("emu->pc before return: 0x%x\nsp: before return: 0x%x\n",emu->pc,emu->sp);
					emu->sp--;
					emu->pc = emu->stack[emu->sp];
					debug_print("emu->pc after return: 0x%x\nsp: after return: 0x%x\n",emu->pc,emu->sp);
					break;
			}
			break;
		case 0x1000: //1NNN: Jumps to address NNN.
			emu->pc = opcode & 0x0FFF;
			break;
		case 0x2000: //2NNN: Calls subroutine at NNN.
			debug_print("emu->pc before call: 0x%x\nsp: before call: 0x%x\n",emu->pc,emu->sp);
			emu->stack[emu->sp] = emu->pc + 2;
			emu->sp++;
			emu->pc = opcode & 0x0FFF;
			debug_print("emu->pc after call: 0x%x\nsp: after call: 0x%x\n",emu->pc,emu->sp);
			break;
		case 0x3000: //3XNN: Skips the next instruction if VX equals NN.
			if (emu->V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				emu->pc += 4;
			else
				emu->pc += 2;
			break;
		case 0x4000: //4XNN: Skips the next instruction if VX doesn't equal NN.
			if (emu->V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				emu->pc += 4;
			else
				emu->pc += 2;
			break;
		case 0x5000: //5XY0: Skips the next instruction if VX equals VY.
			if (emu->V[(opcode & 0x0F00) >> 8] == emu->V[(opcode & 0x00F0) >> 4])
				emu->pc += 4;
			else
				emu->pc += 2;
			break;
		case 0x6000: //6XNN: Sets VX to NN.
			emu->V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			emu->pc += 2;
			break;
		case 0x7000: //7XNN: Adds NN to VX.
			emu->V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			emu->pc += 2;
			break;
		case 0x8000:
			switch(opcode & 0x000F) {
				case 0x0000: //8XY0: Sets VX to the value of VY.
					emu->V[(opcode & 0x0F00) >> 8] = emu->V[(opcode & 0x00F0) >> 4];
					emu->pc += 2;
					break;
				case 0x0001: //8XY1: Sets VX to VX or VY.
					emu->V[(opcode & 0x0F00) >> 8] |= emu->V[(opcode & 0x00F0) >> 4];
					emu->pc += 2;
					break;
				case 0x0002: //8XY2: Sets VX to VX and VY.
					emu->V[(opcode & 0x0F00) >> 8] &= emu->V[(opcode & 0x00F0) >> 4];
					emu->pc += 2;
					break;
				case 0x0003: //8XY3: Sets VX to VX xor VY.
					emu->V[(opcode & 0x0F00) >> 8] ^= emu->V[(opcode & 0x00F0) >> 4];
					emu->pc += 2;
					break;
				case 0x0004: //8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
					if (emu->V[(opcode & 0x0F00) >> 8] + emu->V[(opcode & 0x00F0) >> 4] > 0xFF)
						emu->V[0xF] = 1;
					else
						emu->V[0xF] = 0;
					emu->V[(opcode & 0x0F00) >> 8] = (emu->V[(opcode & 0x0F00) >> 8] + emu->V[(opcode & 0x00F0) >> 4]) % 0xFF;
					emu->pc += 2;
					break;
				case 0x0005: //8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
					if (emu->V[(opcode & 0x0F00) >> 8] - emu->V[(opcode & 0x00F0) >> 4] < 0)
						emu->V[0xF] = 1;
					else
						emu->V[0xF] = 0;
						emu->V[(opcode & 0x0F00) >> 8] = (emu->V[(opcode & 0x0F00) >> 8] - emu->V[(opcode & 0x00F0) >> 4]) % 0xFF;
					emu->pc += 2;
					break;
				case 0x0006: //8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
					emu->V[0xF] = emu->V[(opcode & 0x0F00) >> 8] & 0x1;
					emu->V[(opcode & 0x0F00) >> 8] >>= 1;
					emu->pc += 2;
					break;
				case 0x0007: //8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
					if (emu->V[(opcode & 0x00F0) >> 4] - emu->V[(opcode & 0x0F00) >> 8] < 0)
						emu->V[0xF] = 1;
					else
						emu->V[0xF] = 0;
					emu->V[(opcode & 0x0F00) >> 8] = (emu->V[(opcode & 0x00F0) >> 4] - emu->V[(opcode & 0x0F00) >> 8]) % 0xFF;
					emu->pc += 2;
					break;
				case 0x000E: //8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
					emu->V[0xF] = emu->V[(opcode & 0x0F00) >> 8] & 0x8000;
					emu->V[(opcode & 0x0F00) >> 8] <<= 1;
					emu->pc += 2;
					break;
			}
			break;
		case 0x9000: //9XY0: Skips the next instruction if VX doesn't equal VY.
			if (emu->V[(opcode & 0x0F00) >> 8] != emu->V[(opcode & 0x00F0) >> 4])
				emu->pc += 4;
			else
				emu->pc +=2;
			break;
		case 0xA000: //ANNN: Sets I to the address NNN.
			emu->I = opcode & 0x0FFF;
			emu->pc += 2;
			break;
		case 0xB000: //BNNN: Jumps to the address NNN plus V0.
			emu->pc = (opcode & 0x0FFF) + emu->V[0];
			break;
		case 0xC000: //CXNN: Sets VX to the result of a bitwise and operation on a random number and NN.
			emu->V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			emu->pc += 2;
			break;
		case 0xD000: { //DXYN: Sprites stored in memory at location in index register (I), 8bits wide.
					   //Wraps around the screen. If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero.
					   //All drawing is XOR drawing (i.e. it toggles the screen pixels). Sprites are drawn starting at position VX, VY.
					   //N is the number of 8bit rows that need to be drawn.
					   //If N is greater than 1, second line continues at position VX, VY+1, and so on.
			uint8_t x = emu->V[(opcode & 0x0F00) >> 8];
			uint8_t y = emu->V[(opcode & 0x00F0) >> 4];
			uint8_t h = opcode & 0x000F;
			uint8_t pxls;
			debug_print("XOR at (%d, %d)\n",x,y);
			emu->V[0xF] = 0;
			for (int _y = 0; _y < h; _y++) {
				pxls = emu->mem[emu->I + _y];
				for (int _x = 0; _x < 8; _x++) {
					if ((pxls & (0x80 >> _x)) != 0 && (x+_x>=0 && x+_x<64 && y+_y>=0 && y+_y<32)) {
						if (emu->gfx[(x + _x + ((y + _y) * 64))] == 1)
							emu->V[0xF] = 1;
						emu->gfx[(x + _x + ((y + _y) * 64))] ^= 1;
					}
				}
			}

			emu->updateScreen = true;
			emu->pc += 2;
			break;
		}
		case 0xE000:
			switch(opcode & 0x00FF) {
				case 0x009E: //EX9E: Skips the next instruction if the key stored in VX is pressed.
					if (emu->keys[emu->V[(opcode & 0x0F00) >> 8]])
						emu->pc += 4;
					else
						emu->pc += 2;
					break;
				case 0x00A1: //EXA1: Skips the next instruction if the key stored in VX isn't pressed.
					if (!emu->keys[emu->V[(opcode & 0x0F00) >> 8]])
						emu->pc += 4;
					else
						emu->pc += 2;
					break;
			}
			break;
		case 0xF000:
			switch (opcode & 0x00FF) {
				case 0x0007: //FX07: Sets VX to the value of the delay timer.
					emu->V[(opcode & 0x0F00) >> 8] = emu->delay_timer;
					emu->pc += 2;
					break;
				case 0x000A: //FX0A: A key press is awaited, and then stored in VX.
					if (emu->wait_key >= 0) {
						emu->V[(opcode & 0x0F00) >> 8] = emu->wait_key;
						emu->wait_key = -1;
						emu->pc += 2;
					}
					break;
				case 0x0015: //FX15: Sets the delay timer to VX.
					emu->delay_timer = emu->V[(opcode & 0x0F00) >> 8];
					emu->pc += 2;
					break;
				case 0x0018: //FX18: Sets the sound timer to VX.
					emu->sound_timer = emu->V[(opcode & 0x0F00) >> 8];
					emu->pc += 2;
					break;
				case 0x001E: //FX1E: Adds VX to I.
					emu->I += emu->V[(opcode & 0x0F00) >> 8];
					emu->pc += 2;
					break;
				case 0x0029: //FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
					emu->I = emu->V[(opcode & 0x0F00) >> 8] * 5;
					emu->pc += 2;
					break;
				case 0x0033: //FX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
					emu->mem[emu->I] = ((opcode & 0x0F00) >> 8) / 100;
					emu->mem[emu->I + 1] = (((opcode & 0x0F00) >> 8) / 10) % 10;
					emu->mem[emu->I + 2] = (((opcode & 0x0F00) >> 8) % 100) % 10;
					emu->pc += 2;
					break;
				case 0x0055: //FX55: Stores V0 to VX in memory starting at address I.
					for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
						emu->mem[emu->I + i] = emu->V[i];
					emu->pc += 2;
					break;
				case 0x0065: //FX65: Fills V0 to VX with values from memory starting at address I.
					for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
						emu->V[i] = emu->mem[emu->I + i];
					emu->pc += 2;
					break;
			}
			break;
		default:
			return false;
	}

	return true;
}

void chip8_loadROM(struct chip8* emu, char* ROM_NAME) {
	FILE* rom = fopen(ROM_NAME, "r");
	fread(&(emu->mem)[0x200], 8, 0xFFF - 0x200, rom);
	fclose(rom);
	debug_print("Loaded ROM: %s\n", ROM_NAME);
}

uint8_t chip8_font[] = {
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

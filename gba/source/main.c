#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>

#include "../../core/chip8.h"

#define SCALE			3

void setPixel(int x, int y, int size, u16 color);
//extern uint8_t IBMLogo_ch8[];
//extern int IBMLogo_ch8_size;
//extern uint8_t RushHourHap2006_ch8[];
//extern int RushHourHap2006_ch8_size;
extern uint8_t* Invaders;
extern int Invaders_size;
const uint8_t keymap[] = {5, //KEY_A
						  5, //KEY_B
						  0, //KEY_SELECT
						  0, //KEY_START
						  6, //KEY_RIGHT
						  4, //KEY_LEFT
						  0, //KEY_UP
						  0, //KEY_DOWN
						  0, //KEY_R
						  0, //KEY_L
};
//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	REG_DISPCNT = MODE_3 | BG2_ENABLE;

	struct chip8 emu;
	chip8_init(&emu, false);
	memcpy((emu.mem + 0x200), &Invaders, Invaders_size);

	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);

	while (1) {
		for (int i = 0; i < 25; i++) {
			chip8_step(&emu);
		}

		if (emu.updateScreen) {
			for (int y = 0; y < 32; y++) {
				for (int x = 0; x < 64; x++) {
					if (emu.gfx[(y*64) + x]) {
						setPixel(x*SCALE, y*SCALE, SCALE, 0xFFFF);
					} else {
						setPixel(x*SCALE, y*SCALE, SCALE, 0x0000);
					}
				}
			}
			emu.updateScreen = false;
		}

		scanKeys();
		u16 keys = keysHeld();
		for (int i = 0; i <= 9; i++) {
			emu.keys[keymap[i]] = keys & (1<<i);
		}


		VBlankIntrWait();
	}
}

void setPixel(int x, int y, int size, u16 color) {
	for (int _y = 0; _y < size; _y++) {
		for (int _x = 0; _x < size; _x++) {
			((u16*)VRAM)[((y+_y)*SCREEN_WIDTH) + x+_x] = color;
		}
	}
}

#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_dma.h>

#include "../../core/chip8.h"

#define SCALE		3
#define X_OFFSET	(240 - 64*SCALE)/2 //24
#define Y_OFFSET	(160 - 32*SCALE)/2 //32

void setPixel(int x, int y, int size, u16 color);
//extern uint8_t IBMLogo_ch8[];
//extern int IBMLogo_ch8_size;
//extern uint8_t RushHourHap2006_ch8[];
//extern int RushHourHap2006_ch8_size;
extern uint8_t Invaders[];
extern int Invaders_size;
extern void* __rom_end__;
uint8_t keymap[] = {5, //KEY_A
					5, //KEY_B
					0, //KEY_SELECT
					5, //KEY_START
					6, //KEY_RIGHT
					4, //KEY_LEFT
					5, //KEY_UP
					0, //KEY_DOWN
					6, //KEY_R
					4, //KEY_L
};


typedef struct DMA_REC {
    const void *src;
    void *dst;
    u32 cnt;
} DMA_REC;

#define REG_DMA ((volatile DMA_REC*)0x040000B0)

inline void dma3_fill(void *dst, volatile u32 src, uint count) {
	REG_DMA[3].cnt = 0; // shut off any previous transfer
	REG_DMA[3].src = (const void*)&src;
	REG_DMA[3].dst = dst;
	REG_DMA[3].cnt = count | DMA_SRC_FIXED | DMA_ENABLE;
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {

	REG_DISPCNT = MODE_3 | BG2_ENABLE;

	struct chip8 emu;
	chip8_init(&emu, false);
	memcpy((emu.mem + 0x200), &Invaders, Invaders_size);

	//for (int i = 0; i < 10; i++) {
		//keymap[i] = ((uint8_t*)__rom_end__)[i*2] - 48;
	//}

	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);

	dma3_fill((void*)VRAM, 0x0000, SCREEN_WIDTH * SCREEN_HEIGHT);

	while (1) {
		for (int i = 0; i < 25; i++) {
			chip8_step(&emu);
		}

		if (emu.updateScreen) {
			for (int y = 0; y < 32; y++) {
				for (int x = 0; x < 64; x++) {
					if (emu.gfx[(y*64) + x]) {
						setPixel(x*SCALE + X_OFFSET, y*SCALE + Y_OFFSET, SCALE, 0xFFFF);
					} else {
						setPixel(x*SCALE + X_OFFSET, y*SCALE + Y_OFFSET, SCALE, 0x0000);
					}
				}
			}
			emu.updateScreen = false;
		}

		scanKeys();
		u16 keys = keysHeld();
		memset(emu.keys, 0, 0xF);
		for (int i = 0; i <= 9; i++) {
			if (keys & (1<<i)) {
				emu.keys[keymap[i]] = 1;
			}
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

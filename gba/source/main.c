#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_dma.h>

#include "chip8.h"

#define SCALE		3
#define X_OFFSET	(240 - 64*SCALE)/2 //24
#define Y_OFFSET	(160 - 32*SCALE)/2 //32

void setPixel(int x, int y, int size, u16 color);
extern uint8_t ROM[];
extern uint8_t keymap[];
//const uint8_t keymap[] = {5,5,0,5,6,4,5,0,6,4};

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
	memcpy((emu.mem + 0x200), &ROM, 0xE00);

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

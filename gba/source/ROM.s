	.section .rodata
	.balign 4
	.global keymap
keymap:
	.byte 5, 5, 0, 5, 6, 4, 5, 0, 6, 4
	.byte 0xDE ,0xAD, 0xBE, 0xEF
	.global ROM
ROM:
	.fill 0xE00

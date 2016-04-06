import mmap, sys, os.path, binascii

ROM_MAGIC = b'\xDE\xAD\xBE\xEF'
KEYS_MAGIC = b'\x05\x05\x00\x05\x06\x04\x05\x00\x06\x04'

keynames = ['KEY_A', 'KEY_B', 'KEY_SELECT', 'KEY_START', 'KEY_RIGHT', 'KEY_LEFT', 'KEY_UP', 'KEY_DOWN', 'KEY_R', 'KEY_L']
keymap = []

def error(msg):
    print(msg)
    sys.exit(1)

if len(sys.argv) != 3:
    error('usage: {} [gba rom] [chip 8 rom]'.format(sys.argv[0]))
else:
    if not os.path.isfile(sys.argv[1]):
        error('{} is not a file or does not exist'.format(sys.argv[1]))
    elif not os.path.isfile(sys.argv[2]):
        error('{} is not a file or does not exist'.format(sys.argv[2]))
    gba_f = open(sys.argv[1], 'r+b')
    gba = mmap.mmap(gba_f.fileno(), 0)
    rom_start = gba.find(ROM_MAGIC) + 4
    keys_start = gba.find(KEYS_MAGIC)
    if rom_start == 3 or keys_start == -1:
        error('Invalid {}'.format(sys.argv[1]))
    elif os.path.getsize(sys.argv[2]) > 0xE00:
        error('{} is too large'.format(sys.argv[2]))
    gba.seek(rom_start)
    gba.write(b'\x00' * 0xE00)
    gba.seek(rom_start)
    gba.write(open(sys.argv[2]).read())
    gba.seek(keys_start)
    for key in keynames:
        k = chr(int(raw_input('Enter CHIP-8 key for GBA key {}: '.format(key)), 16))
        gba.write_byte(k)
    gba.close()

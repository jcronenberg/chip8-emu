#include <stdio.h>

#define MEMORYSIZE 4096
#define REGISTERS 16
#define GFXSIZE 64 * 32
#define STACKSIZE 16
#define KEYSIZE 16
#define FONTSETLENGTH 80

unsigned short opcode;

unsigned char memory[4096];

unsigned char V[REGISTERS];

unsigned short I;
unsigned short pc;

unsigned char gfx[GFXSIZE];

unsigned char delayTimer;
unsigned char soundTimer;

unsigned short stack[STACKSIZE];
unsigned short sp;

unsigned char key[KEYSIZE];

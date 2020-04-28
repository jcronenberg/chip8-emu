#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "GL/freeglut.h"
#include "GL/gl.h"

#include "chip8.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

uint8_t screenData[SCREEN_WIDTH][SCREEN_HEIGHT][3];

const int modifier = 20;
int counter = 0;

int drawFlag = 1;

int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;

void drawPixel(int x, int y)
{
    glBegin(GL_QUADS);
	glVertex3f((x * modifier) + 0.0f,     (y * modifier) + 0.0f,	 0.0f);
	glVertex3f((x * modifier) + 0.0f,     (y * modifier) + modifier, 0.0f);
        glVertex3f((x * modifier) + modifier, (y * modifier) + modifier, 0.0f);
	glVertex3f((x * modifier) + modifier, (y * modifier) + 0.0f,	 0.0f);
    glEnd();
}

void updateQuads(unsigned char show[])
{
    for(int y = 0; y < SCREEN_HEIGHT; y++) {
        for(int x = 0; x < SCREEN_WIDTH; x++) {
            if (show[(y * SCREEN_WIDTH) + x])
                glColor3f(1.0f,1.0f,1.0f);
            else
	        glColor3f(0.0f,0.0f,0.0f);	

	    drawPixel(x, y);
        }
    }
}

void reshape_window(GLsizei w, GLsizei h)
{
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);        
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
    
    // Resize quad
    display_width = w;
    display_height = h;
}

void keyboardDown(unsigned char input, int x, int y)
{
    if(input == 27)    // esc
    	exit(0);

    if(input == '1')		key[0x1] = 1;
    else if(input == '2')	key[0x2] = 1;
    else if(input == '3')	key[0x3] = 1;
    else if(input == '4')	key[0xC] = 1;

    else if(input == 'q')	key[0x4] = 1;
    else if(input == 'w')	key[0x5] = 1;
    else if(input == 'e')	key[0x6] = 1;
    else if(input == 'r')	key[0xD] = 1;

    else if(input == 'a')	key[0x7] = 1;
    else if(input == 's')	key[0x8] = 1;
    else if(input == 'd')	key[0x9] = 1;
    else if(input == 'f')	key[0xE] = 1;

    else if(input == 'z')	key[0xA] = 1;
    else if(input == 'x')	key[0x0] = 1;
    else if(input == 'c')	key[0xB] = 1;
    else if(input == 'v')	key[0xF] = 1;

    printf("Pressed key: %c\n", input); //debug
}

void keyboardUp(unsigned char input, int x, int y)
{
    if(input == '1')		key[0x1] = 0;
    else if(input == '2')	key[0x2] = 0;
    else if(input == '3')	key[0x3] = 0;
    else if(input == '4')	key[0xC] = 0;

    else if(input == 'q')	key[0x4] = 0;
    else if(input == 'w')	key[0x5] = 0;
    else if(input == 'e')	key[0x6] = 0;
    else if(input == 'r')	key[0xD] = 0;

    else if(input == 'a')	key[0x7] = 0;
    else if(input == 's')	key[0x8] = 0;
    else if(input == 'd')	key[0x9] = 0;
    else if(input == 'f')	key[0xE] = 0;

    else if(input == 'z')	key[0xA] = 0;
    else if(input == 'x')	key[0x0] = 0;
    else if(input == 'c')	key[0xB] = 0;
    else if(input == 'v')	key[0xF] = 0;

    printf("Released key: %c\n", input); //debug
}

void initializeSystem()
{
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    //Clear everything
    for (int i = 0; i < GFXSIZE; i++)
        gfx[i] = 0;
    for (int i = 0; i < STACKSIZE; i++)
        stack[i] = 0;
    for (int i = 0; i < REGISTERS; i++)
        V[i] = 0;
    for (int i = 0; i < MEMORYSIZE; i++)
        memory[i] = 0;

    //Load fontset
    for (int i = 0; i < FONTSETLENGTH; i++)
        memory[i] = 0;//chip8Fontset[i];
    
    //Reset timers
    delayTimer = 0;
    soundTimer = 0;
}

int loadGame(char *fn)
{
    char *buffer = NULL;
    FILE *readfile;
    long bufsize = 0;

    readfile = fopen(fn, "rb");
    if (readfile == NULL) {
        printf("Error reading file\n");
        return EXIT_FAILURE;
    }

    if (fseek(readfile, 0L, SEEK_END) == 0) {
        bufsize = ftell(readfile);
        if (bufsize == -1) { printf("Error bufsize"); }

        buffer = malloc(sizeof(char) * (bufsize + 1));

        if (fseek(readfile, 0L, SEEK_SET) != 0) { printf("Error with fseek"); }

        size_t newLen = fread(buffer, sizeof(char), bufsize, readfile);

        if (ferror(readfile) != 0) {
            printf("Error reading file");
        } else {
            buffer[newLen++] = '\0'; /* Just to be safe. */
        }
    }

    fclose(readfile);

    for (int i = 0; i < bufsize; i++)
        memory[i + 0x200] = buffer[i];

    free(buffer);
}

void emulateCycle()
{
    //Fetch opcode
    opcode = memory[pc] | memory[pc + 1];

    switch (opcode) {
    case 0xA2F0:
        I = opcode & 0x0FFF;
        pc += 2;
    }
}

void display(void)
{
    if (drawFlag) {
        printf("Drawing Frame!\n"); //debug

        glClear(GL_COLOR_BUFFER_BIT);

        updateQuads(gfx);

        glutSwapBuffers();

        drawFlag = 0;
    }

    if (key[0x1])
        drawFlag = 1;
}

int main(int argc, char **argv)
{

    if (argc > 2) {
        printf("Too many arguments\n");
        return EXIT_FAILURE;
    } else if (argc < 2) {
        printf("Please specify a file to disassemble\n");
        return EXIT_FAILURE;
    }

    initializeSystem();
    if (loadGame(argv[1])) {
        fprintf(stderr, "Loading Game failed");
        return EXIT_FAILURE;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    glutInitWindowSize(display_width, display_height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Chip8 Emulator");

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape_window);

    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    glutMainLoop();
    return 0;
}

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include "GL/freeglut.h"
#include "GL/gl.h"

#include "chip8.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

static int modifier = 20;
static int drawFlag = 1;
static int debugFlag = 0;

static int display_width;
static int display_height;

// Print usage
static void usage()
{
    printf("Chip 8 Emulator:\n");
    printf("  chip8 [OPTIONS] [ROM]\n");
    printf("    -d, --debug: Enable debug mode\n");
}

// Update pixels
static void updateQuads()
{
    for(int a = 0; a < SCREEN_HEIGHT; a++) {
        for(int b = 0; b < SCREEN_WIDTH; b++) {
            if (gfx[(a * SCREEN_WIDTH) + b])
                glColor3f(1.0f,1.0f,1.0f);
            else
	        glColor3f(0.0f,0.0f,0.0f);

            glBegin(GL_QUADS);
            glVertex3f((b * modifier),     (a * modifier),	 0.0f);
            glVertex3f((b * modifier),     (a * modifier) + modifier, 0.0f);
            glVertex3f((b * modifier) + modifier, (a * modifier) + modifier, 0.0f);
            glVertex3f((b * modifier) + modifier, (a * modifier),	 0.0f);
            glEnd();
        }
    }
}

static void reshape_window(GLsizei w, GLsizei h)
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

static void keyboardDown(unsigned char input, int x, int y)
{
    if(input == 27)    // esc
    	exit(EXIT_SUCCESS);

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
}

static void keyboardUp(unsigned char input, int x, int y)
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
}

static void initializeSystem()
{
    fprintf(stderr, "Initializing system with default values\n");
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
    for (int i = 0; i < KEYSIZE; i++)
        key[i] = 0;

    //Load fontset
    for (int i = 0; i < FONTSETLENGTH; i++)
        memory[i] = chip8Fontset[i];

    //Reset timers
    delayTimer = 0;
    soundTimer = 0;

    //Seed rand()
    srand(time(NULL));

    fprintf(stderr, "Finished initializing!\n");
}

static int loadGame(char *fn)
{
    fprintf(stderr, "Loading game: %s\n", fn);

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

    fprintf(stderr, "Finished loading!\n");
    return 0;
}

static int checkDelayTimer(unsigned char timer)
{
    static clock_t before;
    static int newTimerFlag = 1;
    int msec = 0;

    if(newTimerFlag) {
        before = clock();
        newTimerFlag = 0;
    }

    clock_t difference = clock() - before;
    msec = difference * 1000 / CLOCKS_PER_SEC;

    if (msec >= 16) {
        timer--;
        newTimerFlag = 1;
    }

    return timer;
}

static void emulateCycle()
{
    //Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    //Flag to inc pc by the standard value of 2
    //as it is the case most of the time
    int incPC = 1;

    //Declare variables for X and Y and set them
    int X = (opcode & 0x0F00) >> 8;
    int Y = (opcode & 0x00F0) >> 4;

    if (debugFlag) {
      fprintf(stderr, "opcode: 0x%x\n", opcode);
      fprintf(stderr, "V[X]: %x\n", V[X]);
      fprintf(stderr, "V[Y]: %x\n", V[X]);
    }

    //Decode opcode
    switch (opcode & 0xF000) {

    case 0x0000:
        switch (opcode & 0x000F) {

        case 0x0000:
            for (int i = 0; i < GFXSIZE; i++)
                gfx[i] = 0;

            drawFlag = 1;
            break;
        case 0x000E:
            sp--;
            pc = stack[sp];
            break;
        default:
            fprintf(stderr, "Unknown opcode: 0x%x\n", opcode);
        }
        break;
    case 0x1000:
        pc = opcode & 0x0FFF;
        incPC = 0;
        break;
    case 0x2000:
        stack[sp] = pc;
        sp++;
        pc = opcode & 0x0FFF;
        incPC = 0;
        break;
    case 0x3000:
        if (V[X] == (opcode & 0x00FF))
            pc += 2;

        break;
    case 0x4000:
        if (V[X] != (opcode & 0x00FF))
            pc += 2;

        break;
    case 0x5000:
        if (V[X] == V[Y])
            pc += 2;

        break;
    case 0x6000:
        V[X] = opcode & 0x00FF;
        break;
    case 0x7000:
        V[X] += opcode & 0x00FF;
        break;
    case 0x8000:
        switch (opcode & 0x000F) {

        case 0x0000:
            V[X] = V[Y];
            break;
        case 0x0001:
            V[X] |= V[Y];
            break;
        case 0x0002:
            V[X] &= V[Y];
            break;
        case 0x0003:
            V[X] ^= V[Y];
            break;
        case 0x0004:
            if (V[X] + V[Y] > 0xFF)
                V[0xF] = 1;
            else
                V[0xF] = 0;

            V[X] += V[Y];
            break;
        case 0x0005:
            if (V[X] - V[Y] < 0)
                V[0xF] = 0;
            else
                V[0xF] = 1;

            V[X] -= V[Y];
            break;
        case 0x0006:
            V[0xF] = V[X] & 0x1;
            V[X] >>= 1;
            break;
        case 0x0007:
            if (V[X] > V[Y])
                V[0xF] = 0;
            else
                V[0xF] = 1;

            V[X] = V[Y] - V[X];
            break;
        case 0x000E:
            V[0xF] = V[X] >> 7;
            V[X] <<= 1;
            break;
        default:
            fprintf(stderr, "Unknown opcode: 0x%x\n", opcode);
        }
        break;
    case 0x9000:
        if (V[X] != V[Y])
            pc += 2;
        break;
    case 0xA000:
        I = opcode & 0x0FFF;
        break;
    case 0xB000:
        pc = V[0] + (opcode & 0x0FFF);
        incPC = 0;
        break;
    case 0xC000:
        V[X] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
        break;
    case 0xD000:
        {
            unsigned short Vx = V[X];
            unsigned short Vy = V[Y];
            unsigned short height = opcode & 0x000F;
            unsigned short spriterow;
            V[0xF] = 0;

            for (int col = 0; col < height; col++) {
                spriterow = memory[I + col];

                for (int row = 0; row < 8; row++) {

                    if ((spriterow & (0x80 >> row)) != 0) {
                        if (gfx[Vx + row + (Vy + col) * SCREEN_WIDTH % (64 * 32)] == 1)
                            V[0xF] = 1;

                        gfx[Vx + row + (Vy + col) * SCREEN_WIDTH % (64 * 32)] ^= 1;
                    }
                }
            }

            drawFlag = 1;
        }
        break;
    case 0xE000:
        switch (opcode & 0x00FF) {
        case 0x009E:
            if (key[V[X]])
                pc += 2;
            break;
        case 0x00A1:
            if (!key[V[X]])
                pc += 2;
            break;

        default:
            fprintf(stderr, "Unknown opcode: 0x%x\n", opcode);
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF) {
        case 0x0007:
            V[X] = delayTimer;
            break;
        case 0x000A:
            for (int i = 0; i < KEYSIZE; i++) {
                if (key[i]) {
                    V[X] = i;
                    goto keypressed;
                }
            }
            return;
keypressed:
            break;
        case 0x0015:
            delayTimer = V[X];
            break;
        case 0x0018:
            soundTimer = V[X];
            break;
        case 0x001E:
            if (I + V[X] > 0xFFF)
                V[0xF] = 1;
            else
                V[0xF] = 0;

            I += V[X];
            break;
        case 0x0029:
            I = V[X] * 0x5;
            break;
        case 0x0033:
            memory[I] = V[X] / 100;
            memory[I + 1] = (V[X] / 10) % 10;
            memory[I + 2] = V[X] % 10;
            break;
        case 0x0055:
            for (int i = 0; i <= X; i++)
                memory[I + i] = V[i];

            I += X + 1;

            break;
        case 0x0065:
            for (int i = 0; i <= X; i++)
                V[i] = memory[I + i];

            I += X + 1;

            break;

        default:
            fprintf(stderr, "Unknown opcode: 0x%x\n", opcode);
        }
        break;

    default:
        fprintf(stderr, "Unknown opcode: 0x%x\n", opcode);
    }

    //Inc pc if flag is set
    if (incPC)
        pc += 2;

    //Update timers
    if (delayTimer > 0)
        delayTimer = checkDelayTimer(delayTimer);

    if (soundTimer > 0) {
        if (soundTimer > 1)
            fprintf(stderr, "BEEP!\n");

        soundTimer--;
    }
}

static void display(void)
{
    //Initialize before clock to slow down cycle
    clock_t before = clock();

    emulateCycle();

    if (drawFlag) {
        glClear(GL_COLOR_BUFFER_BIT);
        updateQuads();
        glutSwapBuffers();

        drawFlag = 0;
    }

    //Slow down cycle
    int nsec = 0, trigger = 10;
    do {
        clock_t difference = clock() - before;
        nsec = difference * 10000 / CLOCKS_PER_SEC;
    } while (nsec < trigger);

}

int main(int argc, char **argv)
{
    int opt, nonopts;

    int index = 0;
    static struct option longopts[] = {
        {"mod", required_argument, 0, 'm'},
        {"debug", no_argument, 0, 'd'}
    };

    while ((opt = getopt_long(argc, argv, "dm", longopts, &index)) != -1) {
        switch(opt) {
        case 'd':
            debugFlag = 1;
            break;
        case 'm':
            modifier = atoi(optarg);
            break;
        }
    }

    display_width = SCREEN_WIDTH * modifier;
    display_height = SCREEN_HEIGHT * modifier;

    nonopts = argc - optind;

    // Check arguments
    if (nonopts > 1) {
        printf("Too many arguments\n\n");
        usage();
        return EXIT_FAILURE;
    } else if (nonopts < 1) {
        printf("Please specify a ROM to play\n\n");
        usage();
        return EXIT_FAILURE;
    }

    initializeSystem();
    if (loadGame(argv[optind])) {
        fprintf(stderr, "Loading Game failed!\n");
        return EXIT_FAILURE;
    }

    // glut setup
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

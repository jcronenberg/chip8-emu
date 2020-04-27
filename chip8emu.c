#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "GL/freeglut.h"
#include "GL/gl.h"

#define SCREEN_WIDTH 50
#define SCREEN_HEIGHT 50

uint8_t screenData[SCREEN_WIDTH][SCREEN_HEIGHT][3];

const int modifier = 10;
int counter = 0;


int display_width = SCREEN_WIDTH * 10;
int display_height = SCREEN_HEIGHT * 10;

void drawPixel(int x, int y)
{
    glBegin(GL_QUADS);
	glVertex3f((x * modifier) + 0.0f,     (y * modifier) + 0.0f,	 0.0f);
	glVertex3f((x * modifier) + 0.0f,     (y * modifier) + modifier, 0.0f);
        glVertex3f((x * modifier) + modifier, (y * modifier) + modifier, 0.0f);
	glVertex3f((x * modifier) + modifier, (y * modifier) + 0.0f,	 0.0f);
    glEnd();
}

void updateQuads(bool show)
{
    for(int y = 0; y < SCREEN_HEIGHT; ++y) {
        for(int x = 0; x < SCREEN_WIDTH; ++x) {
            if (show)
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

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    updateQuads(counter % 2);
    counter++;

    glutSwapBuffers();

    usleep(100000);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(SCREEN_WIDTH * modifier, SCREEN_HEIGHT * modifier);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Chip8 Emulator");
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape_window);

    glutMainLoop();
    return 0;
}

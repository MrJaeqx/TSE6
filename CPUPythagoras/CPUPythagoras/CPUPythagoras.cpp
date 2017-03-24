#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "bitmap_image.hpp"
#include "pythagoras_frame.h"
#include <windows.h>
#include <CL\cl.h>
#include <CL\cl_gl.h>
#include <stdlib.h>
#include <GL\freeglut.h>

#define WINDOW_SIZE 700

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	pytharos_fractal(-1.6, 5, 1.6, 5, 0.5); //call he recursive function

	printf("DONE\n");
	glFlush();
}


int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);
	glutCreateWindow("Pythagoras fractal");

	glClearColor(0.0, 0.0, 0.0, 0.0);         // background
	glMatrixMode(GL_PROJECTION);              // setup viewing projection
	glLoadIdentity();
	gluOrtho2D(-10.0, 10.0, -10.0, 10.0);

	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}
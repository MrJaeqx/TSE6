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

float windowBounds = 10;

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	LARGE_INTEGER freq, start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	pytharos_fractal(-1.6, 8.28, 1.6, 8.28, 0.5, 1); //call he recursive function

	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	windowBounds = windowBounds * .9;

	//printf("Elapsed time to calculate fractal: %f msec\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);
	printf("%f\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-windowBounds, windowBounds, -windowBounds, windowBounds);

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
	gluOrtho2D(-windowBounds, windowBounds, -windowBounds, windowBounds);

	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}
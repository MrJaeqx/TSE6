#include "pythagoras_frame.h"
#include <math.h>
#include <GL\freeglut.h>

#define WINDOW_SIZE 700

#define ANGLE 60

void pytharos_fractal(float topLeftx, float topLefty, float topRightx, float topRighty, float n)
{
	//1st point						top left corner
	float Q1x = topLeftx;
	float Q1y = topLefty;

	//2nd point						top right corner
	float Q2x = topRightx;
	float Q2y = topRighty;

	//3rd point						bottom right corner
	float Q3x = Q2x - (Q1y - Q2y);
	float Q3y = Q2y + (Q1x - Q2x);

	//4th point						bottom left corner
	float Q4x = (Q1x - Q2x) + Q3x;
	float Q4y = (Q1y - Q2y) + Q3y;

	//display filled square
	glBegin(GL_POLYGON);
	glColor3f(n*n/1.5, n*n / 2, n*n / 2.5);
	glVertex3f(Q1x, Q1y, 0.0);
	glVertex3f(Q2x, Q2y, 0.0);
	glVertex3f(Q3x, Q3y, 0.0);
	glVertex3f(Q4x, Q4y, 0.0);
	glEnd();

	/*//display square outline
	glColor3f(n, n, n);
	glBegin(GL_LINES);
	glVertex3f(Q1x, Q1y, 0.0);
	glVertex3f(Q2x, Q2y, 0.0);
	glVertex3f(Q2x, Q2y, 0.0);
	glVertex3f(Q3x, Q3y, 0.0);
	glVertex3f(Q3x, Q3y, 0.0);
	glVertex3f(Q4x, Q4y, 0.0);
	glVertex3f(Q4x, Q4y, 0.0);
	glVertex3f(Q1x, Q1y, 0.0);
	glEnd();
	//*/


	//calculate begining of next square
	float ACx = (Q3x - Q1x) / 2;	//half of the diagonal
	float ACy = (Q3y - Q1y) / 2;
	float newx = ACx + Q4x;		// point of iscoceles right triangle
	float newy = ACy + Q4y;		// off of the bottom of the square


								//length of a side of the square
	float dist = sqrt(((Q1x - Q2x)*(Q1x - Q2x)) + ((Q1y - Q2y)*(Q1y - Q2y)));

	// 1/700 = x/20
	//20/700 = 1 px?
	if (dist > (20.0 / WINDOW_SIZE)) { //if the square is larger than 1 pixel
		pytharos_fractal(Q4x, Q4y, newx, newy, n + 0.03); //left recursion
		pytharos_fractal(newx, newy, Q3x, Q3y, n + 0.08); //right recursion
	}
}
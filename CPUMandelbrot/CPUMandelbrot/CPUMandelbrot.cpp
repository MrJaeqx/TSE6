#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "bitmap_image.hpp"
#include "mandelbrot_frame.h"
#include <windows.h>

#define WIDTH 3840
#define HEIGHT 2160
#define OFFSET_X 0
#define OFFSET_Y 0
#define ZOOMFACTOR 800
#define MAX_ITERATIONS 4
#define COLORTABLE_SIZE 1024 

mandelbrot_color colortable2[COLORTABLE_SIZE];

void create_colortable() 
{
	// Initialize color table values
	for(unsigned int i = 0; i < COLORTABLE_SIZE; i++)
	{
		if (i < 64) {
			mandelbrot_color color_entry = {0, 0, (5*i+20<255)?5*i+20:255};
			colortable2[i] = color_entry;
		}

		else if (i < 128) {
			mandelbrot_color color_entry = {0, 2*i, 255};
			colortable2[i] = color_entry;
		}

		else if (i < 512) {
			mandelbrot_color color_entry = {0, (i/4<255)?i/4:255, (i/4<255)?i/4:255};
			colortable2[i] = color_entry;
		}

		else if (i < 768) {
			mandelbrot_color color_entry = {0, (i/4<255)?i/4:255, (i/4<255)?i/4:255};
			colortable2[i] = color_entry;
		}

		else {
			mandelbrot_color color_entry = {0,(i/10<255)?i/10:255,(i/10<255)?i/10:255};
			colortable2[i] = color_entry;
		}
	}	
}

int main() {

	remove("fractal_output.bmp");

	// Create the colortable and fill it with colors
	create_colortable();

	// Create an empty image
	bitmap_image image(WIDTH, HEIGHT);
	mandelbrot_color * p = (mandelbrot_color *)image.data();

	// Get current time before calculating the fractal
	LARGE_INTEGER freq, start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	// Calculate the fractal
	mandelbrot_frame((float)OFFSET_X, (float)OFFSET_Y, (float)1/ZOOMFACTOR, MAX_ITERATIONS, p, colortable2, WIDTH, HEIGHT);
	
	// Get current time after calculating the fractal
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	// Print elapsed time
	printf("Elapsed time to calculate fractal: %f msec\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);
	printf("Press ENTER to continue...\n");
	getchar();

	// Write image to file
	image.save_image("fractal_output.bmp");

	// Show image in mspaint
	WinExec("mspaint fractal_output.bmp", SW_MAXIMIZE);

	return 0;
}
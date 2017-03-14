#include "colortable.h"

void create_colortable(int size, mandelbrot_color * colortable)
{
	/* Initialize color table values */
	for (unsigned int i = 0; i < size; i++)
	{
		if (i < 64) {
			mandelbrot_color color_entry = { 255, 0, (5 * i + 20<255) ? 5 * i + 20 : 255 };
			colortable[i] = color_entry;
		}

		else if (i < 128) {
			mandelbrot_color color_entry = { 0, 2 * i, 255 };
			colortable[i] = color_entry;
		}

		else if (i < 512) {
			mandelbrot_color color_entry = { 0, (i / 4<255) ? i / 4 : 255, (i / 4<255) ? i / 4 : 255 };
			colortable[i] = color_entry;
		}

		else if (i < 768) {
			mandelbrot_color color_entry = { 0, (i / 4<255) ? i / 4 : 255, (i / 4<255) ? i / 4 : 255 };
			colortable[i] = color_entry;
		}

		else {
			mandelbrot_color color_entry = { 0,(i / 10<255) ? i / 10 : 255,(i / 10<255) ? i / 10 : 255 };
			colortable[i] = color_entry;
		}
	}
}

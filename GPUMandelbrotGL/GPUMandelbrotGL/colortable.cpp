#include "colortable.h"

void create_colortable(int size, mandelbrot_color * colortable)
{
	/* Initialize color table values */
	for (unsigned int i = 0; i < size; i++)
	{
		if (i < 64) {
			mandelbrot_color color_entry = { 0, 0, (5 * i + 20<255) ? 5 * i + 20 : 255 };
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

void create_colortable_2(int size, mandelbrot_color * colortable)
{
	/* Initialize color table values */
	for (unsigned int i = 0; i < size; i++)
	{
		if (i < 64) {
			mandelbrot_color color_entry = { 0, 0, (5 * i + 20<255) ? 5 * i + 20 : 255 };
			colortable[i] = color_entry;
		}


		if (i < 64) {
			mandelbrot_color color_entry = { 0 ,0 ,i * 2 };								/* 0x0000 to 0x007E */
			colortable[i] = color_entry;
		}
		else if (i < 128) {
			mandelbrot_color color_entry = { 0 ,0 ,(((i - 64) * 128) / 126) + 128 };	/* 0x0080 to 0x00C0 */
			colortable[i] = color_entry;
		}
		else if (i < 256) {
			mandelbrot_color color_entry = { 0 ,0 , (((i - 128) * 62) / 127) + 193 };	/* 0x00C1 to 0x00FF */
			colortable[i] = color_entry;
		}
		else if (i < 512) {
			mandelbrot_color color_entry = { 0, (((i - 256) * 62) / 255) + 1, 255 };    /* 0x01FF to 0x3FFF */
			colortable[i] = color_entry;
		}
		else if (i < 1024) {
			mandelbrot_color color_entry = { 0, (((i - 512) * 63) / 511) + 1, 255 };	/* 0x40FF to 0x7FFF */
			colortable[i] = color_entry;
		}
		else if (i < 2048) {
			mandelbrot_color color_entry = { 0, (((i - 1024) * 63) / 1023) + 128, 255 };	/* 0x80FF to 0xBFFF */
			colortable[i] = color_entry;
		}
		else if (i < 4096) {
			mandelbrot_color color_entry = { 0, (((i - 2048) * 63) / 2047) + 192, 255 };	/* 0xC0FF to 0xFFFF */
			colortable[i] = color_entry;
		}
		else {
			mandelbrot_color color_entry = { 0, 255, 255 };
			colortable[i] = color_entry;
		}

	}
}
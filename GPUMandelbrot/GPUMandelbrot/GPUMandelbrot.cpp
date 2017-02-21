#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "bitmap_image.hpp"
#include "mandelbrot_frame.h"
#include "opencl_utils.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <CL/cl.h>
#include <stdlib.h>

#define WIDTH 3840
#define HEIGHT 2160
#define OFFSET_X 0
#define OFFSET_Y 0
#define ZOOMFACTOR 800
#define MAX_ITERATIONS 8192
#define COLORTABLE_SIZE 1024

#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

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
	// Define GPU parameters
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;
	
	cl_mem dev_bitmap = NULL;
	cl_mem dev_colortable = NULL;
	cl_mem dev_params = NULL;

	unsigned int params[7] = {WIDTH, HEIGHT, OFFSET_X, OFFSET_Y, ZOOMFACTOR, MAX_ITERATIONS, COLORTABLE_SIZE};
	char fileName[] = "./mandelbrot.cl";

	// Create the colortable and fill it with colors
	create_colortable();

	// Remove previous bitmap
	remove("fractal_output.bmp");

	// Create an empty image
	bitmap_image image(WIDTH, HEIGHT);
	mandelbrot_color * p = (mandelbrot_color *)image.data();

	/* Get Platform and Device Info */
	size_t infoSize;
	char* info;

	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	ret = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 0, NULL, &infoSize);
	checkError(ret, "Couldn't get platform and device info");

	info = (char*)malloc(infoSize);
	ret = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, infoSize, info, NULL);
	checkError(ret, "Couldn't get platform attribute value");

	printf("Running on %s\n\n", info);

	

	/* Create OpenCL Context */
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	checkError(ret, "Couldn't create context");

	/* Create command queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	checkError(ret, "Couldn't create commandqueue");

	/* Allocate memory for arrays on the Compute Device */
	dev_params = clCreateBuffer(context, CL_MEM_READ_ONLY, 7 * sizeof(int), NULL, &ret);
	checkError(ret, "Couldn't create params on device");
	dev_bitmap = clCreateBuffer(context, CL_MEM_READ_WRITE, WIDTH * HEIGHT * sizeof(mandelbrot_color), NULL, &ret);
	checkError(ret, "Couldn't create bitmap on device");
	dev_colortable = clCreateBuffer(context, CL_MEM_READ_ONLY, COLORTABLE_SIZE * sizeof(mandelbrot_color), NULL, &ret);
	checkError(ret, "Couldn't create colortable on device");


	/* Copy arrays A and B from host memory to Compute Devive */
	ret = clEnqueueWriteBuffer(command_queue, dev_params, CL_TRUE, 0, 7 * sizeof(int), params, 0, NULL, NULL);
	checkError(ret, "Couldn't write params on device");
	ret = clEnqueueWriteBuffer(command_queue, dev_colortable, CL_TRUE, 0, COLORTABLE_SIZE * sizeof(mandelbrot_color), colortable2, 0, NULL, NULL);
	checkError(ret, "Couldn't write colortable on device");

	/* Create kernel program */
	program = build_program(context, device_id, fileName);
	checkError(ret, "Couldn't compile");

	/* Create OpenCL kernel from the compiled program */
	kernel = clCreateKernel(program, "mandelbrot", &ret);
	printf("kernel created\n");
	checkError(ret, "Couldn't create kernel");

	/* Set OpenCL kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&dev_params);
	checkError(ret, "Couldn't set arg dev_params");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&dev_bitmap);
	checkError(ret, "Couldn't set arg dev_bitmap");
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&dev_colortable);
	checkError(ret, "Couldn't set arg dev_colortable");

	// Get current time before calculating the fractal
	LARGE_INTEGER freq, start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	/* Activate OpenCL kernel on the Compute Device */
	size_t globalSize[] = { WIDTH , HEIGHT };
	size_t localSize[] = { 800 , 1 };
	clEnqueueNDRangeKernel(command_queue,
		kernel,
		2,           // it’s a 2-dimensional matrix 
		NULL,
		globalSize,  // 3x3 work items globally
		NULL,   // 3x3 work item per group 
		0,
		NULL,
		NULL);

	/* Add blocking element */
	clFinish(command_queue);

	// Get current time after calculating the fractal
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	/* Transfer result array C back to host */
	ret = clEnqueueReadBuffer(command_queue, dev_bitmap, CL_TRUE, 0, WIDTH * HEIGHT * sizeof(mandelbrot_color), p, 0, NULL, NULL);
	checkError(ret, "Couldn't get ...");

	/* Add blocking element */
	clFinish(command_queue);

	/* Finalization */
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(dev_bitmap);
	ret = clReleaseMemObject(dev_colortable);
	ret = clReleaseMemObject(dev_params);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	// Print elapsed time
	printf("Elapsed time to calculate fractal: %f msec\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);
	printf("Press ENTER to continue...\n");

	// Write image to file
	image.save_image("fractal_output.bmp");

	// Show image in mspaint
	WinExec("mspaint fractal_output.bmp", SW_MAXIMIZE);

	getchar();

	return 0;
}
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "bitmap_image.hpp"
#include "mandelbrot_frame.h"
#include "colortable.h"
#include "opencl_utils.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <CL/cl.h>
#include <stdlib.h>
#include <GL\freeglut.h>
#include "OpenGl_functions.h"


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

cl_int ret;

cl_kernel kernel = NULL;

cl_command_queue command_queue = NULL;

cl_mem dev_bitmap = NULL;
cl_mem dev_colortable = NULL;
cl_mem dev_params = NULL;

mandelbrot_color * p;


void display() {
	/* Clear all pixels */
	glFlush();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Draw polygon (rectangle) with corners at (0.0, 0.0, 0.0) and (0.5, 0.5, 0.0) */	
	glColor3f(0.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
		glVertex3f(-1.0, -1.0, 0.0);
		glVertex3f(1.0, -1.0, 0.0);
		glVertex3f(1.0, 1.0, 0.0);
		glVertex3f(-1.0, 1.0, 0.0);
	glEnd();

	/* don't wait!
	* start processing buffered OpenGL routines
	*/
	//glFlush();


	/*
	// Acquire OpenCL access to the texture   
	clEnqueueAcquireGLObjects(queue, 1, &cl_tex, 0, NULL, NULL);

	clSetKernelArg(..., ..., sizeof(cl_mem), (void *)&cl_tex);
	clEnqueueNDRangeKernel(......);

	// Give the texture back to OpenGL
	clEnqueueReleaseGLObjects(queue, 1, &cl_tex, 0, NULL, NULL);

	//Wait until object is actually released by OpenCL
	clFinish(queue);

	// Draw quad, with the texture mapped on it in OPENGL
	......

	// Request another redisplay of the window
	glutPostRedisplay();*/

	/* Set OpenCL kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&dev_params);
	checkError(ret, "Couldn't set arg dev_params");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&dev_bitmap);
	checkError(ret, "Couldn't set arg dev_bitmap");
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&dev_colortable);
	checkError(ret, "Couldn't set arg dev_colortable");

	/* Activate OpenCL kernel on the Compute Device */
	size_t globalSize[] = { WIDTH , HEIGHT };
	size_t localSize[] = { 800 , 1 };
	clEnqueueNDRangeKernel(command_queue,
		kernel,
		2,			// 2D matrix 
		NULL,
		globalSize,
		NULL,		// local size (NULL = auto) 
		0,
		NULL,
		NULL);

	/* Add blocking element */
	clFinish(command_queue);

	/* Transfer result array C back to host */
	ret = clEnqueueReadBuffer(command_queue, dev_bitmap, CL_TRUE, 0, WIDTH * HEIGHT * sizeof(mandelbrot_color), p, 0, NULL, NULL);
	checkError(ret, "Couldn't get ...");

	/* Add blocking element */
	clFinish(command_queue);
}


int main(int argc, char** argv) {
	/* Define GPU parameters */
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_program program = NULL;
	cl_platform_id platform_id = NULL;
	
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	
	size_t infoSize;
	unsigned int params[7] = {WIDTH, HEIGHT, OFFSET_X, OFFSET_Y, ZOOMFACTOR, MAX_ITERATIONS, COLORTABLE_SIZE};	
	char* info;
	char fileName[] = "./mandelbrot.cl";

	/* Create window*/
	glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_SINGLE);
	glutInitWindowSize(500, 500);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow("Hello world");
	glutDisplayFunc(display);

	/* Init GL */
	//init_gl(500, 500);

	/* Create the colortable and fill it with colors */
	create_colortable(COLORTABLE_SIZE, colortable2);

	/* Create an empty image */
	bitmap_image image(WIDTH, HEIGHT);
	p = (mandelbrot_color *)image.data();

	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	checkError(ret, "Couldn't get platform ids");
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	checkError(ret, "Couldn't get device ids");
	ret = clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &infoSize);
	checkError(ret, "Couldn't get device info size");
	info = (char*)malloc(infoSize);
	ret = clGetDeviceInfo(device_id, CL_DEVICE_NAME, infoSize, info, NULL);
	checkError(ret, "Couldn't get device info value");
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

	/* Copy arrays from host memory to Compute Devive */
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

	glutMainLoop();

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

	/* Print stuff */
	printf("Press ENTER to continue...\n");

	getchar();

	return 0;
}
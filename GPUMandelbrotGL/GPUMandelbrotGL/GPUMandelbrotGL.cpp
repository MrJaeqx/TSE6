#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "bitmap_image.hpp"
#include "mandelbrot_frame.h"
#include "colortable.h"
#include "opencl_utils.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <CL\cl.h>
#include <CL\cl_gl.h>
#include <stdlib.h>
#include <GL\freeglut.h>
#include "OpenGl_functions.h"

#define WIDTH 500
#define HEIGHT 500
#define OFFSET_X -0.2414798
#define OFFSET_Y -.5521
#define MAX_ITERATIONS 8192
#define COLORTABLE_SIZE 4096

mandelbrot_color colortable[COLORTABLE_SIZE];

cl_int ret;

cl_kernel kernel = NULL;

cl_command_queue command_queue = NULL;

cl_mem dev_colortable = NULL;
cl_mem dev_params = NULL;
cl_mem dev_texture = NULL;
cl_mem dev_stepsize = NULL;

mandelbrot_color * p;

GLuint g_texture;

int now, previous;
float stepsize = 0.01;
float params[6] = { WIDTH, HEIGHT, OFFSET_X, OFFSET_Y, MAX_ITERATIONS, COLORTABLE_SIZE };


void display() {
	/* Clear all pixels */
	glFlush();
	
	/* Wait for GL finish */
	glFinish();

	/* Calculate stepsize */
	now = glutGet(GLUT_ELAPSED_TIME);
	stepsize *= pow(.95, (now - previous) / 100.0);
	previous = now;

	/* Write stepsize to device */
	ret = clEnqueueWriteBuffer(command_queue, dev_stepsize, CL_TRUE, 0, sizeof(float), &stepsize, 0, NULL, NULL);
	checkError(ret, "Couldn't write stepsize on device");

	/* Aquie GL texture */
	ret = clEnqueueAcquireGLObjects(command_queue, 1, &dev_texture, 0, NULL, NULL);
	checkError(ret, "Couldn't acquire CL object");

	/* Set OpenCL kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&dev_params);
	checkError(ret, "Couldn't set arg dev_params");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&dev_texture);
	checkError(ret, "Couldn't set arg dev_bitmap");
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&dev_colortable);
	checkError(ret, "Couldn't set arg dev_colortable");
	ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&dev_stepsize);
	checkError(ret, "Couldn't set arg dev_colortable");

	/* Activate OpenCL kernel on the Compute Device */
	size_t globalSize[] = { WIDTH , HEIGHT };
	size_t localSize[] = { 800 , 1 };
	
	/* Run kernel */
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, globalSize, NULL, 0, NULL, NULL);
	checkError(ret, "Couldn't execute kernel");

	/* Release texture */
	ret = clEnqueueReleaseGLObjects(command_queue, 1, &dev_texture, 0, NULL, NULL);
	checkError(ret, "Couldn't release GL object");

	/* Add blocking element */
	clFinish(command_queue);

	/* Use OpenGL_functions to draw quad */
	draw_quad();

	/* Request another redisplay of the window */
	glutPostRedisplay();
}


int main(int argc, char** argv) {
	/* Define GPU parameters */
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_program program = NULL;
	cl_platform_id platform_id[2];// = NULL;
	
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;

	size_t infoSize;
	//params = {WIDTH, HEIGHT, OFFSET_X, OFFSET_Y, ZOOMFACTOR, MAX_ITERATIONS, COLORTABLE_SIZE};	
	char* info;
	char* infoPlat;
	char fileName[] = "./mandelbrot.cl";

	/* Create the colortable and fill it with colors */
	create_colortable_2(COLORTABLE_SIZE, colortable);

	/* Create window*/
	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(300, 300);
	glutCreateWindow("Mandelbrot");
	glutDisplayFunc(display);

	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(2, platform_id, &ret_num_platforms);
	checkError(ret, "Couldn't get platform ids");

	for (int i = 0; i < (int)ret_num_platforms; i++) {
		ret = clGetPlatformInfo(platform_id[i], CL_PLATFORM_NAME, 0, NULL, &infoSize);
		checkError(ret, "Couldn't get device info size");
		infoPlat = (char*)malloc(infoSize);
		ret = clGetPlatformInfo(platform_id[i], CL_PLATFORM_NAME, infoSize, infoPlat, NULL);
		checkError(ret, "Couldn't get device info value");
		printf("Device [%d], %s\n", i, infoPlat);
	}

	ret = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);
	checkError(ret, "Couldn't get device ids");
	ret = clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &infoSize);
	checkError(ret, "Couldn't get device info size");
	info = (char*)malloc(infoSize);
	ret = clGetDeviceInfo(device_id, CL_DEVICE_NAME, infoSize, info, NULL);
	checkError(ret, "Couldn't get device info value");
	printf("Running on %s\n\n", info);

	/* Create OpenCL Context */
	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, reinterpret_cast<cl_context_properties>(wglGetCurrentContext()),
		CL_WGL_HDC_KHR, reinterpret_cast<cl_context_properties>(wglGetCurrentDC()),
		0
	};

	context = clCreateContext(properties, 1, &device_id, NULL, NULL, &ret);
	checkError(ret, "Couldn't create context");

	/* Create command queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	checkError(ret, "Couldn't create commandqueue");

	/* Allocate memory for arrays on the Compute Device */
	dev_stepsize = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float), NULL, &ret);
	checkError(ret, "Couldn't create stepsize on device");
	dev_params = clCreateBuffer(context, CL_MEM_READ_ONLY, 6 * sizeof(float), NULL, &ret);
	checkError(ret, "Couldn't create params on device");
	dev_colortable = clCreateBuffer(context, CL_MEM_READ_ONLY, COLORTABLE_SIZE * sizeof(mandelbrot_color), NULL, &ret);
	checkError(ret, "Couldn't create colortable on device");

	/* Copy arrays from host memory to Compute Devive */
	ret = clEnqueueWriteBuffer(command_queue, dev_params, CL_TRUE, 0, 6 * sizeof(float), params, 0, NULL, NULL);
	checkError(ret, "Couldn't write params on device");
	ret = clEnqueueWriteBuffer(command_queue, dev_colortable, CL_TRUE, 0, COLORTABLE_SIZE * sizeof(mandelbrot_color), colortable, 0, NULL, NULL);
	checkError(ret, "Couldn't write colortable on device");

	/* Create kernel program */
	program = build_program(context, device_id, fileName);
	checkError(ret, "Couldn't compile");

	/* Create OpenCL kernel from the compiled program */
	kernel = clCreateKernel(program, "mandelbrot", &ret);
	printf("kernel created\n\n");
	checkError(ret, "Couldn't create kernel");

	/* Create texture in OpenGL using OpenGL_functions */
	g_texture = init_gl(500, 500);

	/* Map texture to OpenCL */
	dev_texture = clCreateFromGLTexture2D(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, g_texture, &ret);
	checkError(ret, "Couldn't create texture");

	/* Run glut window main loop */
	glutMainLoop();

	/* Finalization */
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(dev_texture);
	ret = clReleaseMemObject(dev_colortable);
	ret = clReleaseMemObject(dev_params);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	/* Print stuff */
	printf("Press ENTER to continue...\n");

	getchar();

	return 0;
}
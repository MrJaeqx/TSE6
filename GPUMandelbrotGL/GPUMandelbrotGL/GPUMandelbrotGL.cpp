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
#define OFFSET_X 0
#define OFFSET_Y 0
#define ZOOMFACTOR 300
#define MAX_ITERATIONS 8192
#define COLORTABLE_SIZE 1024

mandelbrot_color colortable[COLORTABLE_SIZE];

cl_int ret;

cl_kernel kernel = NULL;

cl_command_queue command_queue = NULL;

cl_mem dev_colortable = NULL;
cl_mem dev_params = NULL;
cl_mem dev_texture = NULL;

mandelbrot_color * p;

GLuint g_texture;


void display() {
	/* Clear all pixels */
	glFlush();
	
	/* Wait for GL finish */
	glFinish();

	ret = clEnqueueAcquireGLObjects(command_queue, 1, &dev_texture, 0, NULL, NULL);
	checkError(ret, "Couldn't acquire CL object");

	/* Set OpenCL kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&dev_params);
	checkError(ret, "Couldn't set arg dev_params");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&dev_texture);
	checkError(ret, "Couldn't set arg dev_bitmap");
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&dev_colortable);
	checkError(ret, "Couldn't set arg dev_colortable");

	/* Activate OpenCL kernel on the Compute Device */
	size_t globalSize[] = { WIDTH , HEIGHT };
	size_t localSize[] = { 800 , 1 };
	
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, globalSize, NULL, 0, NULL, NULL);
	checkError(ret, "Couldn't execute kernel");

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
	cl_platform_id platform_id = NULL;
	
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;

	size_t infoSize;
	unsigned int params[7] = {WIDTH, HEIGHT, OFFSET_X, OFFSET_Y, ZOOMFACTOR, MAX_ITERATIONS, COLORTABLE_SIZE};	
	char* info;
	char fileName[] = "./mandelbrot.cl";

	/* Create the colortable and fill it with colors */
	create_colortable(COLORTABLE_SIZE, colortable);

	/* Create window*/
	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(300, 300);
	glutCreateWindow("Mandelbrot");
	glutDisplayFunc(display);

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
	dev_params = clCreateBuffer(context, CL_MEM_READ_ONLY, 7 * sizeof(int), NULL, &ret);
	checkError(ret, "Couldn't create params on device");
	dev_colortable = clCreateBuffer(context, CL_MEM_READ_ONLY, COLORTABLE_SIZE * sizeof(mandelbrot_color), NULL, &ret);
	checkError(ret, "Couldn't create colortable on device");

	/* Copy arrays from host memory to Compute Devive */
	ret = clEnqueueWriteBuffer(command_queue, dev_params, CL_TRUE, 0, 7 * sizeof(int), params, 0, NULL, NULL);
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
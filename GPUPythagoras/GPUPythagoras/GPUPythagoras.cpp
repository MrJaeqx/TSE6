#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "pythagoras_frame.h"
#include "opencl_utils.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <CL\cl.h>
#include <CL\cl_gl.h>
#include <stdlib.h>
#include <GL\freeglut.h>
#include "OpenGl_functions.h"

#define WINDOW_SIZE 700

pythagoras_coords startPos = { -1.6, 8.28, 1.6, 8.28 };

float windowBounds = 10;

cl_int ret;

cl_kernel kernel = NULL;

cl_command_queue command_queue = NULL;

cl_mem dev_input = NULL;
cl_mem dev_output_l = NULL;
cl_mem dev_output_r = NULL;

GLuint g_texture;

int now, previous;
float stepsize = 0.01;

void drawQuad(pythagoras_coords co) {
	// calc bottom right corner
	float Q3x = co.topRightx - (co.topLefty - co.topRighty);
	float Q3y = co.topRighty + (co.topLeftx - co.topRightx);

	// calc bottom left corner
	float Q4x = (co.topLeftx - co.topRightx) + Q3x;
	float Q4y = (co.topLefty - co.topRighty) + Q3y;

	// Draw quad
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(co.topLeftx, co.topLefty, 0.0);
	glVertex3f(co.topRightx, co.topRighty, 0.0);
	glVertex3f(co.topRightx, co.topRighty, 0.0);
	glVertex3f(Q3x, Q3y, 0.0);
	glVertex3f(Q3x, Q3y, 0.0);
	glVertex3f(Q4x, Q4y, 0.0);
	glVertex3f(Q4x, Q4y, 0.0);
	glVertex3f(co.topLeftx, co.topLefty, 0.0);
	glEnd();
}


void display() {
	pythagoras_coords outputL;
	pythagoras_coords outputR;
	pythagoras_coords startPosL = { -1.6, 8.28, 1.6, 8.28 };
	pythagoras_coords startPosR = { -1.6, 8.28, 1.6, 8.28 };

	// Wait for GL finish
	glFinish();

	glClear(GL_COLOR_BUFFER_BIT);

	drawQuad(startPos);

	/*LARGE_INTEGER freq, start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	printf("Elapsed time to calculate fractal: %f msec\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);
	*/	

	// Write stepsize to device
	ret = clEnqueueWriteBuffer(command_queue, dev_input, CL_TRUE, 0, sizeof(pythagoras_coords), &startPosL, 0, NULL, NULL);
	checkError(ret, "Couldn't write stepsize on device");

	// Set OpenCL kernel arguments
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&dev_input);
	checkError(ret, "Couldn't set arg dev_params");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&dev_output_l);
	checkError(ret, "Couldn't set arg dev_bitmap");
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&dev_output_r);
	checkError(ret, "Couldn't set arg dev_bitmap");

	// Activate OpenCL kernel on the Compute Device
	size_t globalSize[] = { 1 , 1 };
	//size_t localSize[] = { 700 , 1 };

	// Run kernel
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, globalSize, NULL, 0, NULL, NULL);
	checkError(ret, "Couldn't execute kernel");

	// Get output of calculation
	ret = clEnqueueReadBuffer(command_queue, dev_output_l, CL_TRUE, 0, sizeof(pythagoras_coords), &outputL, 0, NULL, NULL);
	checkError(ret, "Couldn't get output");
	// Get output of calculation
	ret = clEnqueueReadBuffer(command_queue, dev_output_r, CL_TRUE, 0, sizeof(pythagoras_coords), &outputR, 0, NULL, NULL);
	checkError(ret, "Couldn't get output");

	// Add blocking element
	clFinish(command_queue);

	printf("1: Output L: %f, %f, %f, %f\n", outputL.topLeftx, outputL.topLefty, outputL.topRightx, outputL.topRighty);
	printf("1: Output R: %f, %f, %f, %f\n", outputR.topLeftx, outputR.topLefty, outputR.topRightx, outputR.topRighty);

	drawQuad(outputL);
	drawQuad(outputR);





	// Write stepsize to device
	ret = clEnqueueWriteBuffer(command_queue, dev_input, CL_TRUE, 0, sizeof(pythagoras_coords), &outputL, 0, NULL, NULL);
	checkError(ret, "Couldn't write stepsize on device");

	// Set OpenCL kernel arguments
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&dev_input);
	checkError(ret, "Couldn't set arg dev_params");
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&dev_output_l);
	checkError(ret, "Couldn't set arg dev_bitmap");
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&dev_output_r);
	checkError(ret, "Couldn't set arg dev_bitmap");

	// Run kernel
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, globalSize, NULL, 0, NULL, NULL);
	checkError(ret, "Couldn't execute kernel");

	// Get output of calculation
	ret = clEnqueueReadBuffer(command_queue, dev_output_l, CL_TRUE, 0, sizeof(pythagoras_coords), &outputL, 0, NULL, NULL);
	checkError(ret, "Couldn't get output");
	// Get output of calculation
	ret = clEnqueueReadBuffer(command_queue, dev_output_r, CL_TRUE, 0, sizeof(pythagoras_coords), &outputR, 0, NULL, NULL);
	checkError(ret, "Couldn't get output");

	// Add blocking element
	clFinish(command_queue);

	printf("2: Output L: %f, %f, %f, %f\n", outputL.topLeftx, outputL.topLefty, outputL.topRightx, outputL.topRighty);
	printf("2: Output R: %f, %f, %f, %f\n", outputR.topLeftx, outputR.topLefty, outputR.topRightx, outputR.topRighty);

	drawQuad(outputL);
	drawQuad(outputR);


	///* Request another redisplay of the window */

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-windowBounds, windowBounds, -windowBounds, windowBounds);

	glFlush();
	//glutPostRedisplay();
}


int main(int argc, char** argv) {
	// Define GPU parameters
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_program program = NULL;
	cl_platform_id platform_id[2];// = NULL;
	
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;

	size_t infoSize;	
	char* info;
	char* infoPlat;
	char fileName[] = "./pythagoras.cl";

	// Create window
	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);
	glutCreateWindow("Pythagoras fractal");

	// Get Platform and Device Info
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

	// Create OpenCL Context
	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, reinterpret_cast<cl_context_properties>(wglGetCurrentContext()),
		CL_WGL_HDC_KHR, reinterpret_cast<cl_context_properties>(wglGetCurrentDC()),
		0
	};

	// Create context
	context = clCreateContext(properties, 1, &device_id, NULL, NULL, &ret);
	checkError(ret, "Couldn't create context");

	// Create command queue 
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	checkError(ret, "Couldn't create commandqueue");

	// Allocate memory for arrays on the Compute Device
	dev_input = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(pythagoras_coords), NULL, &ret);
	checkError(ret, "Couldn't create stepsize on device");
	dev_output_l = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(pythagoras_coords), NULL, &ret);
	checkError(ret, "Couldn't create params on device");
	dev_output_r = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(pythagoras_coords), NULL, &ret);
	checkError(ret, "Couldn't create params on device");

	// Copy arrays from host memory to Compute Devive
	ret = clEnqueueWriteBuffer(command_queue, dev_input, CL_TRUE, 0, sizeof(pythagoras_coords), &startPos, 0, NULL, NULL);
	checkError(ret, "Couldn't write params on device");

	// Create kernel program
	program = build_program(context, device_id, fileName);
	checkError(ret, "Couldn't compile");

	// Create OpenCL kernel from the compiled program
	kernel = clCreateKernel(program, "pythagoras", &ret);
	printf("kernel created\n\n");
	checkError(ret, "Couldn't create kernel");

	// Create texture in OpenGL using OpenGL_functions
	glClearColor(0.0, 0.0, 0.0, 0.0);         // background
	glMatrixMode(GL_PROJECTION);              // setup viewing projection
	glLoadIdentity();
	gluOrtho2D(-windowBounds, windowBounds, -windowBounds, windowBounds);
	glutDisplayFunc(display);

	// Run glut window main loop
	glutMainLoop();

	// Finalization
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(dev_input);
	ret = clReleaseMemObject(dev_output_l);
	ret = clReleaseMemObject(dev_output_r);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	// Print stuff
	printf("Press ENTER to continue...\n");

	getchar();

	return 0;
}
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "opencl_utils.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <CL/cl.h>
#include <stdlib.h>

int main() {
	/* Define GPU parameters */
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;
	
	cl_mem dev_g_data = NULL;

	size_t infoSize;	
	char* info;
	int numbers[] = { 10, 1, 8, -1, 0, -2, 3, 5, -2, -3, 2, 7, 0, 11, 0, 2 };
	int ret_number[2];
	char fileName[] = "./kernel_3.cl";

	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	checkError(ret, "Couldn't get platform ids");
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	checkError(ret, "Couldn't get device ids");
	ret = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 0, NULL, &infoSize);
	checkError(ret, "Couldn't get platform info");
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
	dev_g_data = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(numbers), NULL, &ret);
	checkError(ret, "Couldn't create global data on device");

	/* Get current time before calculating the fractal */
	LARGE_INTEGER freq, start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	/* Copy arrays from host memory to Compute Devive */
	ret = clEnqueueWriteBuffer(command_queue, dev_g_data, CL_TRUE, 0, sizeof(numbers), numbers, 0, NULL, NULL);
	checkError(ret, "Couldn't write numbers on device");

	/* Get current time after calculating the fractal */
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	/* Print elapsed time */
	printf("Elapsed time to copy to GPU:   %f msec\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);

	/* Create kernel program */
	program = build_program(context, device_id, fileName);
	checkError(ret, "Couldn't compile");

	/* Create OpenCL kernel from the compiled program */
	kernel = clCreateKernel(program, "kernel_3", &ret);
	checkError(ret, "Couldn't create kernel");

	/* Set OpenCL kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&dev_g_data);
	checkError(ret, "Couldn't set arg dev_g_data");
	ret = clSetKernelArg(kernel, 1, 8 * sizeof(int), NULL);
	checkError(ret, "Couldn't set arg dev_l_data");

	/* Get current time before calculating the fractal */
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	/* Activate OpenCL kernel on the Compute Device */
	size_t globalSize[] = { sizeof(numbers) / sizeof(numbers[0]) };
	size_t localSize[] = { 8 };

	// TODO: SPLIT INTO MORE KERNELS DEPENDING ON ARRAY SIZE

	clEnqueueNDRangeKernel(command_queue,
		kernel,
		1,			// 1D matrix 
		NULL,
		globalSize,
		localSize,		// local size (NULL = auto) 
		0,
		NULL,
		NULL);

	/* Add blocking element */
	clFinish(command_queue);

	/* Get current time after calculating the fractal */
	QueryPerformanceCounter(&end);

	/* Print elapsed time */
	printf("Elapsed time for kernel calls: %f msec\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);

	/* Get current time before calculating the fractal */
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	/* Transfer result array C back to host */
	ret = clEnqueueReadBuffer(command_queue, dev_g_data, CL_TRUE, 0, sizeof(ret_number), ret_number, 0, NULL, NULL);
	checkError(ret, "Couldn't get return");

	/* Add blocking element */
	clFinish(command_queue);

	/* Get current time after calculating the fractal */
	QueryPerformanceCounter(&end);

	/* Print elapsed time */
	printf("Elapsed time to read from GPU: %f msec\n\n", (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000.0);

	/* Finalization */
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(dev_g_data);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	/* Print result */
	printf("Kernel used:  %s\n", fileName);
	printf("Array length: %d\n\n", (int)sizeof(numbers) / sizeof(numbers[0]));
	printf("Result:       %d\n\n", ret_number[0]);
	printf("Press ENTER to continue...\n");

	getchar();

	return 0;
}
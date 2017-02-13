// OpenCL_1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <CL/cl.h>
#include <stdlib.h>

#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)


int main(void)
{
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_mem memobj = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;
	cl_mem AonDevice = NULL;
	cl_mem BonDevice = NULL;
	cl_mem ConDevice = NULL;

	unsigned int A[3][3] = { { 0, 1, 2 },{ 3, 4, 5 },{ 6, 7, 8 } },
		B[3][3] = { { 0, 1, 2 },{ 3, 4, 5 },{ 6, 7, 8 } },
		C[3][3];

	FILE *fp;
	char fileName[] = "./addArray.cl";
	char *source_str;
	size_t source_size;

	/* Load the source code containing the kernel */
	fp = fopen(fileName, "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	printf("loaded kernel\n");

	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

	/* Create OpenCL Context */
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

	/* Create command queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	/* Allocate memory for arrays on the Compute Device */
	AonDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, 3 * 3 * sizeof(int), NULL, &ret);
	BonDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, 3 * 3 * sizeof(int), NULL, &ret);
	ConDevice = clCreateBuffer(context, CL_MEM_READ_WRITE, 3 * 3 * sizeof(int), NULL, &ret);

	/* Copy arrays A and B from host memory to Compute Devive */
	ret = clEnqueueWriteBuffer(command_queue, AonDevice, CL_TRUE, 0, 3 * 3 * sizeof(int), A, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, BonDevice, CL_TRUE, 0, 3 * 3 * sizeof(int), B, 0, NULL, NULL);

	/* Create kernel program */
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

	/* Compile the program Just In Time to GPU machine code */
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	/* Create OpenCL kernel from the compiled program */
	kernel = clCreateKernel(program, "addArray", &ret);

	printf("kernel created\n");

	/* Set OpenCL kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&AonDevice);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&BonDevice);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&ConDevice);

	/* Activate OpenCL kernel on the Compute Device */
	size_t globalSize[] = { 3,3 };
	size_t localSize[] = { 3,3 };
	clEnqueueNDRangeKernel(	command_queue, 
							kernel, 
							2,           // it’s a 2-dimensional matrix 
							NULL, 
							globalSize,  // 3x3 work items globally
							localSize,   // 3x3 work item per group 
							0,
							NULL,
							NULL);


	/* Transfer result array C back to host */
	ret = clEnqueueReadBuffer(command_queue, ConDevice, CL_TRUE, 0, 3 * 3 * sizeof(int), C, 0, NULL, NULL);

	/* Display Result */
	printf("C:\n");
	for (int x = 0; x<3; x++)  // loop 3 times for three lines
	{
		for (int y = 0; y<3; y++)  // loop for the three elements on the line
		{
			std::cout << C[x][y];  // display the current element out of the array
		}
		std::cout << std::endl;  // when the inner loop is done, go to a new line
	}

	/* Finalization */
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(memobj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	free(source_str);

	getchar();

	return 0;

}


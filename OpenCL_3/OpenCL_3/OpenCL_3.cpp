#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "opencl_utils.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <CL/cl.h>
#include <stdlib.h>

int getInt()
{
	int num;
	std::cin.operator >> (num);
	return num;
}

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

	LARGE_INTEGER timeWriteFreq, timeWriteStart, timeWriteEnd, timeCalcFreq, timeCalcStart, timeCalcEnd, timeReadFreq, timeReadStart, timeReadEnd;

	size_t infoSize;
	char* info;
	
	int ret_number[2];
	char fileName[] = "./kernel_3.cl";
	char kernelName[] = "kernel_3";

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

	/* Get user input */
	printf("Select kernel (1-3)\n");
	int kernelSelect = getInt();
	printf("\nSelect array size\n");
	int arraySize = getInt();


	/* Generate array */
	printf("\nGenerating array, this could take some time...\n");
	int* numbers = new int[arraySize];
	for (int i = 0; i < arraySize; i++) {
		numbers[i] = i;
	}

	/* Select given kernel */
	if (kernelSelect == 1) {
		strcpy(fileName, "./kernel_1.cl");
		strcpy(kernelName, "kernel_1");
	}
	else if (kernelSelect == 2) {
		strcpy(fileName, "./kernel_2.cl");
		strcpy(kernelName, "kernel_2");
	}
	else {
		strcpy(fileName, "./kernel_3.cl");
		strcpy(kernelName, "kernel_3");
	}

	/* Create OpenCL Context */
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	checkError(ret, "Couldn't create context");

	/* Create command queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	checkError(ret, "Couldn't create commandqueue");

	/* Allocate memory for arrays on the Compute Device */
	dev_g_data = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(numbers) * arraySize, NULL, &ret);
	checkError(ret, "Couldn't create global data on device");

	/* Get current time before write */
	QueryPerformanceFrequency(&timeWriteFreq);
	QueryPerformanceCounter(&timeWriteStart);

	/* Copy arrays from host memory to Compute Devive */
	ret = clEnqueueWriteBuffer(command_queue, dev_g_data, CL_TRUE, 0, sizeof(numbers) * arraySize, numbers, 0, NULL, NULL);
	checkError(ret, "Couldn't write numbers on device");

	/* Get current time after write */
	QueryPerformanceCounter(&timeWriteEnd);

	/* Create kernel program */
	program = build_program(context, device_id, fileName);
	checkError(ret, "Couldn't compile");

	/* Create OpenCL kernel from the compiled program */
	kernel = clCreateKernel(program, kernelName, &ret);
	checkError(ret, "Couldn't create kernel");

	/* Get max workgroup size */
	size_t maxWorkgroupSize;
	ret = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkgroupSize, NULL);
	checkError(ret, "Couldn't get kernel info");

	/* Get user input */
	printf("\nSelect workgroup size (1-%d)\n", maxWorkgroupSize);
	int workgroupSize = getInt();

	/* Set OpenCL kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&dev_g_data);
	checkError(ret, "Couldn't set arg dev_g_data");
	ret = clSetKernelArg(kernel, 1, workgroupSize * sizeof(int), NULL);
	checkError(ret, "Couldn't set arg dev_l_data");

	/* Get current time before calculating the fractal */
	QueryPerformanceFrequency(&timeCalcFreq);
	QueryPerformanceCounter(&timeCalcStart);

	/* Activate OpenCL kernel on the Compute Device */
	
	size_t globalSize[] = { arraySize };
	size_t localSize[] = { workgroupSize };

	/* Split into kernels depending on array size*/
	//DEBUG int i = 0;
	while (globalSize[0] >= localSize[0] && localSize[0] > 1) {
		//DEBUG printf("\nIteration:   %d\n", i);
		//DEBUG printf("Global size: %d\n", globalSize[0]);
		//DEBUG printf("Local size:  %d\n", localSize[0]);*/

		/* Execute kernel */
		ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, globalSize, localSize, 0, NULL, NULL);
		checkError(ret, "Couldn't run kernel");

		/* Calculate new global size for next iteration */
		if (strcmp(kernelName, "kernel_3") == 0) {
			globalSize[0] = globalSize[0] / localSize[0] / 2;
		} else {
			globalSize[0] = globalSize[0] / localSize[0];
		}
		if (globalSize[0] < localSize[0]) {
			localSize[0] = globalSize[0];
		}		
		//DEBUG i++;
	}

	/* Add blocking element */
	clFinish(command_queue);

	/* Get current time after calculating the fractal */
	QueryPerformanceCounter(&timeCalcEnd);

	/* Get current time before calculating the fractal */
	QueryPerformanceFrequency(&timeReadFreq);
	QueryPerformanceCounter(&timeReadStart);

	/* Transfer result array C back to host */
	ret = clEnqueueReadBuffer(command_queue, dev_g_data, CL_TRUE, 0, sizeof(ret_number), ret_number, 0, NULL, NULL);
	checkError(ret, "Couldn't get return");

	/* Add blocking element */
	clFinish(command_queue);

	/* Get current time after calculating the fractal */
	QueryPerformanceCounter(&timeReadEnd);

	/* Finalization */
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(dev_g_data);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	/* Print result */
	printf("\nElapsed time to copy to GPU:   %f msec\n", (double)(timeWriteEnd.QuadPart - timeWriteStart.QuadPart) / timeWriteFreq.QuadPart * 1000.0);
	printf("Elapsed time for kernel calls: %f msec\n", (double)(timeCalcEnd.QuadPart - timeCalcStart.QuadPart) / timeCalcFreq.QuadPart * 1000.0);
	printf("Elapsed time to read from GPU: %f msec\n\n", (double)(timeReadEnd.QuadPart - timeReadStart.QuadPart) / timeReadFreq.QuadPart * 1000.0);
	printf("Kernel used:  %s\n", fileName);
	printf("Array length: %d\n\n", arraySize);
	printf("Result:       %d\n\n", ret_number[0]);
	printf("Press ENTER to continue...\n");

	getchar();
	getchar();

	return 0;
}
// OpenCL_2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <CL/OpenCL.h>
#include "opencl_utils.h"

int main()
{
	unsigned int i, j;
	cl_int err;
	char* info;
	size_t infoSize;
	cl_uint platformCount;
	cl_platform_id *platforms;
	const char* attributeNames[5] = { "Name", "Vendor", "Version", "Profile", "Extensions" };
	const cl_platform_info attributeTypes[5] =
	{ CL_PLATFORM_NAME, CL_PLATFORM_VENDOR, CL_PLATFORM_VERSION, CL_PLATFORM_PROFILE, CL_PLATFORM_EXTENSIONS };
	const int attributeCount = sizeof(attributeNames) / sizeof(char*);

	// get platform count
	err = clGetPlatformIDs(5, NULL, &platformCount);
	checkError(err, "Couldn't get amount of platforms");

	// get all platforms
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * platformCount);
	err = clGetPlatformIDs(platformCount, platforms, NULL);
	checkError(err, "Couldn't identify platforms");

	// for each platform print all attributes
	for (i = 0; i < platformCount; i++) {

		printf("\n %d. Platform \n", i);

		for (j = 0; j < attributeCount; j++) {

			// get platform attribute value size
			err = clGetPlatformInfo(platforms[i], attributeTypes[j], 0, NULL, &infoSize);
			checkError(err, "Couldn't get platform attribute value size");

			info = (char*)malloc(infoSize);

			// get platform attribute value
			err = clGetPlatformInfo(platforms[i], attributeTypes[j], infoSize, info, NULL);
			checkError(err, "Couldn't get platform attribute value");

			printf("  %d.%d %-11s: %s\n", i, j, attributeNames[j], info);
			free(info);

		}

		printf("\n");

	}

	free(platforms);
	getchar();
	return 0;

}


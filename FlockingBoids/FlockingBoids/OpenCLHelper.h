#pragma once
#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

bool CheckOpenCLError(cl_int errNum, const char *errMsg)
{
	if (errNum != CL_SUCCESS)
	{
		std::cerr << errMsg << std::endl;
		return false;
	}
	return true;
}

void cleanupOpenCLModels(cl_context context, cl_uint *numOfDevices, cl_command_queue *commandQueues, cl_program program, cl_kernel kernel) {
	if (kernel != 0)
		clReleaseKernel(kernel);

	if (program != 0)
		clReleaseProgram(program);

	if (commandQueues != 0) {
		for (int i = 0; i < *numOfDevices; i++) {
			clReleaseCommandQueue(commandQueues[i]);
		}
	}

	if (context != 0)
		clReleaseContext(context);

	*numOfDevices = 0;

}

cl_context createOpenCLContext(cl_device_type deviceType) {
	cl_int errNum;
	cl_platform_id platform[4];
	cl_uint numPlatforms;
	errNum = clGetPlatformIDs(4, &platform[0], &numPlatforms);
	if (!CheckOpenCLError(errNum, "Failed to find any OpenCL platforms.")) {
		return NULL;
	}
	if (numPlatforms <= 0) {
		std::cerr << "Failed to find any OpenCL platforms." << std::endl;
		return NULL;
	}
	std::cout << std::endl << numPlatforms << " platforms in total" << std::endl;

	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform[0],
		0
	};

	// Create a context and command queue on the device
	cl_context context = clCreateContextFromType(contextProperties, deviceType, NULL, NULL, &errNum);
	if (!CheckOpenCLError(errNum, "Failed to create an OpenCL GPU or CPU context.")) {
		return NULL;
	}

	return context;
}

cl_uint countOpenlCLDevices(cl_context context) {
	cl_int errNum;
	size_t retSize;
	cl_uint numOfDevices;
	errNum = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(numOfDevices), (void *)&numOfDevices, &retSize);
	if (!CheckOpenCLError(errNum, "Could not get context info!")) {
		numOfDevices = 0;
		return NULL;
	}
	std::cout << std::endl << "There are " << numOfDevices << " devices." << std::endl;
	return numOfDevices;
}

cl_device_id *retrieveOpenCLDevices(cl_context context, cl_uint numOfDevices) {
	cl_int errNum;
	size_t retSize;

	// Get list of devices
	cl_device_id *devices = (cl_device_id *)malloc(numOfDevices * sizeof(cl_device_id));
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, numOfDevices * sizeof(cl_device_id), (void *)devices, &retSize);
	if (!CheckOpenCLError(errNum, "Could not get devices!")) {
		switch (errNum) {
		case CL_INVALID_CONTEXT:
			std::cerr << " (CL_INVALID_CONTEXT)";
			break;
		case CL_INVALID_VALUE:
			std::cerr << " (CL_INVALID_VALUE)";
			break;
		case CL_OUT_OF_RESOURCES:
			std::cerr << " (CL_OUT_OF_RESOURCES)";
			break;
		case CL_OUT_OF_HOST_MEMORY:
			std::cerr << " (CL_OUT_OF_HOST_MEMORY)";
			break;
		default:
			break;
		}
		std::cerr << " size = " << numOfDevices * sizeof(cl_device_id) << ";" << retSize << std::endl;
		return NULL;
	}

	return devices;
}

bool printOpenCLDeviceInfo(cl_device_id device) {
	cl_int errNum;
	cl_device_type devType;
	size_t retSize;

	std::cout << "	" << device << ": ";

	// device type
	errNum = clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), (void *)&devType, &retSize);
	if (!CheckOpenCLError(errNum, "ERROR getting device info!")) {
		return false;
	}

	std::cout << " type " << devType << ":";
	if (devType & CL_DEVICE_TYPE_CPU)
		std::cout << " CPU";
	if (devType & CL_DEVICE_TYPE_GPU)
		std::cout << " GPU";
	if (devType & CL_DEVICE_TYPE_ACCELERATOR)
		std::cout << " accelerator";
	if (devType & CL_DEVICE_TYPE_DEFAULT)
		std::cout << " default";

	// device name
	char devName[1024];
	errNum = clGetDeviceInfo(device, CL_DEVICE_NAME, 1024, (void *)devName, &retSize);
	if (!CheckOpenCLError(errNum, "ERROR getting device name!")) {
		return false;
	}
	std::cout << " name=<" << devName << ">" << std::endl;
}

cl_command_queue createOpenCLCommandQueue(cl_context context, cl_device_id device) {
	cl_int errNum;

	cl_command_queue queue = clCreateCommandQueue(context, device, 0, &errNum);
	if (!CheckOpenCLError(errNum, "ERROR creating command queue!")) {
		return NULL;
	}
	return queue;
}

cl_program createOpenCLProgram(cl_context context, cl_uint numOfDevices, cl_device_id *devices, const char* fileName) {
	cl_int errNum;

	struct stat buf;
	std::ifstream kernelFile(fileName, std::ios::in);
	if (!kernelFile.is_open()) {
		std::cerr << "Failed to open kernel file for reading: " << fileName << std::endl;
		return NULL;
	}

	std::ostringstream oss;
	oss << kernelFile.rdbuf();

	std::string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&srcStr, NULL, &errNum);
	if (!CheckOpenCLError(errNum, "ERROR creating program!")) {
		return NULL;
	}

	errNum = clBuildProgram(program, numOfDevices, devices, NULL, NULL, NULL);
	if (!CheckOpenCLError(errNum, "ERROR building program!")) {
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
		std::cerr << "Error in kernel: " << std::endl;
		std::cerr << buildLog;
		clReleaseProgram(program);
		return NULL;
	}

	return program;
}

cl_kernel createOpenCLKernel(cl_program program, const char* kernelFunction) {
	cl_int errNum;
	cl_kernel kernel = clCreateKernel(program, kernelFunction, &errNum);
	if (!CheckOpenCLError(errNum, "ERROR creating kernel!")) {
		return NULL;
	}

	return kernel;
}
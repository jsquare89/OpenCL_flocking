// Created by Jarred Jardine

#include "Flocking.h"
#include "Errors.h"
#include <stdlib.h>
#include <CL\cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define FLOCK_SIZE (300)


bool CheckOpenCLError(cl_int errNum, const char *errMsg)
{
	if (errNum != CL_SUCCESS)
	{
		std::cerr << errMsg << std::endl;
		return false;
	}
	return true;
}

Flocking::Flocking() : _screenWidth(1280),
	_screenHeight(720),
	_time(0.0f),
	_window(nullptr),
	_gameState(GameState::PLAY)
{
	// OpenCL setup
	cl_int errNum;
	cl_platform_id platform;
	cl_uint numPlatforms;
	errNum = clGetPlatformIDs(1, &platform, &numPlatforms);
	if (!CheckOpenCLError(errNum, "Failed to find any OpenCL platforms.")) {
		return;
	}
	if (numPlatforms <= 0) {
		std::cerr << "Failed to find any OpenCL platforms." << std::endl;
		return;
	}
	std::cout << std::endl << numPlatforms << " platforms in total" << std::endl;

	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform,
		0
	};

	// Create a context and command queue on the device
	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ALL, NULL, NULL, &errNum);
	if (!CheckOpenCLError(errNum, "Failed to create an OpenCL GPU or CPU context.")) {
		return;
	}


	// Get all the CPU and GPU devices
	size_t retSize;
	errNum = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(numOfDevices), (void *)&numOfDevices, &retSize);
	if (!CheckOpenCLError(errNum, "Could not get context info!")) {
		return;
	}
	std::cout << std::endl << "There are " << numOfDevices << " devices." << std::endl;

	// Get list of devices
	devices = (cl_device_id *)malloc(numOfDevices * sizeof(cl_device_id));
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
		return;
	}


	// Get device information for each device and assign command queues
	command_queues = (cl_command_queue *)malloc(numOfDevices * sizeof(cl_command_queue));

	cl_device_type devType;
	std::cout << std::endl << "Devices:" << std::endl;
	for (int i = 0; i < numOfDevices; i++) {
		std::cout << "	" << devices[i] << ": ";

		// device type
		errNum = clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(cl_device_type), (void *)&devType, &retSize);
		if (!CheckOpenCLError(errNum, "ERROR getting device info!")) {
			free(devices);
			return;
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
		errNum = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 1024, (void *)devName, &retSize);
		if (!CheckOpenCLError(errNum, "ERROR getting device name!")) {
			free(devices);
			return;
		}
		std::cout << " name=<" << devName << ">" << std::endl;


		// create command queue for each device
		command_queues[i] = clCreateCommandQueue(context, devices[i], 0, &errNum);
		if (!CheckOpenCLError(errNum, "ERROR creating command queue!")) {
			free(devices);
			if (context != 0) clReleaseContext(context);
			return;
		}

	}
	std::cout << std::endl;


	// create program
	const char* fileName = "./flocking.cl";
	struct stat buf;
	std::ifstream kernelFile(fileName, std::ios::in);
	if (!kernelFile.is_open()) {
		std::cerr << "Failed to open kernel file for reading: " << fileName << std::endl;

		if (command_queues != 0) {
			for (int i = 0; i < numOfDevices; i++) {
				clReleaseCommandQueue(command_queues[i]);
			}
		}

		if (context != 0)
			clReleaseContext(context);

		return;
	}

	std::ostringstream oss;
	oss << kernelFile.rdbuf();

	std::string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	program = clCreateProgramWithSource(context, 1, (const char**)&srcStr, NULL, &errNum);
	if (!CheckOpenCLError(errNum, "ERROR creating program!")) {
		if (command_queues != 0) {
			for (int i = 0; i < numOfDevices; i++) {
				clReleaseCommandQueue(command_queues[i]);
			}
		}

		if (context != 0)
			clReleaseContext(context);

		return;
	}

	errNum = clBuildProgram(program, numOfDevices, devices, NULL, NULL, NULL);
	if (!CheckOpenCLError(errNum, "ERROR building program!")) {
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
		std::cerr << "Error in kernel: " << std::endl;
		std::cerr << buildLog;
		clReleaseProgram(program);

		if (command_queues != 0) {
			for (int i = 0; i < numOfDevices; i++) {
				clReleaseCommandQueue(command_queues[i]);
			}
		}

		if (context != 0)
			clReleaseContext(context);

		return;
	}

	// create kernel
	kernel = clCreateKernel(program, "hello_kernel", &errNum);
	if (!CheckOpenCLError(errNum, "ERROR creating kernel!")) {
		if (program != 0)
			clReleaseProgram(program);

		if (command_queues != 0) {
			for (int i = 0; i < numOfDevices; i++) {
				clReleaseCommandQueue(command_queues[i]);
			}
		}

		if (context != 0)
			clReleaseContext(context);

		return;
	}

	// testing code
	int array_size = 10;
	float *result = new float[array_size];
	float *a = new float[array_size];
	float *b = new float[array_size];
	for (int i = 0; i < array_size; i++) {
		a[i] = (float)i;
		b[i] = (float)(i * 2);
	}

	// create memory objects for kernel arguments
	//buffers[0] = clCreateBuffer(context, CL_MEM_READ_ONLY, FLOCK_SIZE * sizeof(cl_uint)
	buffers[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, array_size * sizeof(float), a, NULL);
	buffers[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, array_size * sizeof(float), b, NULL);
	buffers[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, array_size * sizeof(float), NULL, NULL);

	if (buffers[0] == NULL || buffers[1] == NULL || buffers[2] == NULL) {
		std::cerr << "ERROR creating buffers" << std::endl;

		if (kernel != 0)
			clReleaseKernel(kernel);

		if (program != 0)
			clReleaseProgram(program);

		if (command_queues != 0) {
			for (int i = 0; i < numOfDevices; i++) {
				clReleaseCommandQueue(command_queues[i]);
			}
		}

		if (context != 0)
			clReleaseContext(context);

		delete[] b;
		delete[] a;
		delete[] result;

		return;
	}

	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffers[0]) ||
		clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffers[1]) ||
		clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffers[2]);
	if (!CheckOpenCLError(errNum, "ERROR setting kernel arguments")) {
		if (kernel != 0)
			clReleaseKernel(kernel);

		if (program != 0)
			clReleaseProgram(program);

		if (command_queues != 0) {
			for (int i = 0; i < numOfDevices; i++) {
				clReleaseCommandQueue(command_queues[i]);
			}
		}

		if (context != 0)
			clReleaseContext(context);

		delete[] b;
		delete[] a;
		delete[] result;

		return;
	}


	//errNum = clEnqueueReadBuffer(command_queues);
}


Flocking::~Flocking()
{
}


void Flocking::run()
{
	initSystems();
	
	_pflock = new Flock();
	SetupFlock();

	gameLoop();
}

void Flocking::initSystems()
{
	//Initialize SDL
	SDL_Init(SDL_INIT_EVERYTHING);

	_window = SDL_CreateWindow("Flocking", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, SDL_WINDOW_OPENGL);

	if (_window == nullptr)
	{
		fatalError("SDL Window could not be created!");
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(_window);
	if (glContext == nullptr)
	{
		fatalError("SDL_GL context could not be create!");
	}

	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		fatalError("Could not initialize glew!");
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	glClearColor(0.8f, 0.8f, 1.0f, 1.0f);

	initShaders();
}

void Flocking::initShaders()
{
	_colorProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
	_colorProgram.addAttribute("vertexPosition");
	_colorProgram.addAttribute("vertexColor");
	_colorProgram.linkShaders();

}

void Flocking::gameLoop()
{
	while (_gameState != GameState::EXIT)
	{
		processInput();
		_time += 0.01;
		_pflock->Run();
		Render();
	}
}
void Flocking::processInput()
{
	SDL_Event evnt;

	while (SDL_PollEvent(&evnt))
	{
		switch (evnt.type)
		{
		case SDL_QUIT:
			_gameState = GameState::EXIT;
			break;
		case SDL_MOUSEMOTION:
			// std::cout << evnt.motion.x << " " << evnt.motion.y << std::endl;
			break;
		}
	}
}

void Flocking::SetupFlock()
{
	for (int i = 0; i < FLOCK_SIZE; i++)
	{
		Boid* boid = new Boid();
		_pflock->AddBoids(*boid);
	}
}

void Flocking::Render()
{
	// Set the base depth to 1.0
	glClearDepth(1.0);
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_colorProgram.use();

	GLuint timeLocation = _colorProgram.getUniformLocation("time");
	glUniform1f(timeLocation, _time);

	_pflock->Render();

	_colorProgram.unuse();

	glEnd();

	// Swap buffer and draw everything to the screen.
	SDL_GL_SwapWindow(_window);
}
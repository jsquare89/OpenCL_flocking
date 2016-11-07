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
#define KERNEL_FILE ("./flocking.cl")
#define KERNEL_FUNCTION ("hello_kernel")


//class CTiming {
//public:
//	CTiming() {}
//	~CTiming() {}
//
//	void start() { gettimeofday(&tvBegin, NULL); }
//	void end() { gettimeofday(&tvEnd, NULL); }
//	bool diff(int &seconds, int &useconds) {
//		long int diff = (tvEnd.tv_usec + 1000000 * tvEnd.tv_sec) -
//			(tvBegin.tv_usec + 1000000 * tvBegin.tv_sec);
//		seconds = diff / 1000000;
//		useconds = diff % 1000000;
//		return (diff < 0) ? true : false;
//	}
//private:
//	struct timeval, tvBegin, tvEnd, tvDiff;
//};

bool CheckOpenCLError(cl_int errNum, const char *errMsg)
{
	if (errNum != CL_SUCCESS)
	{
		std::cerr << errMsg << std::endl;
		return false;
	}
	return true;
}

void cleanupOpenCLModels(cl_context context, cl_uint numOfDevices, cl_command_queue *commandQueues, cl_program program, cl_kernel kernel) {
	if (kernel != 0)
		clReleaseKernel(kernel);

	if (program != 0)
		clReleaseProgram(program);

	if (commandQueues != 0) {
		for (int i = 0; i < numOfDevices; i++) {
			clReleaseCommandQueue(commandQueues[i]);
		}
	}

	if (context != 0) 
		clReleaseContext(context);

}

cl_context createOpenlCLContext() {
	cl_int errNum;
	cl_platform_id platform;
	cl_uint numPlatforms;
	errNum = clGetPlatformIDs(1, &platform, &numPlatforms);
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
		(cl_context_properties)platform,
		0
	};

	// Create a context and command queue on the device
	cl_context context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ALL, NULL, NULL, &errNum);
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

	clCreateKernel(program, kernelFunction, &errNum);
	if (!CheckOpenCLError(errNum, "ERROR creating kernel!")) {
		return NULL;
	}
}

// for testing
bool runOpenCLHelloWorld(cl_context context, cl_uint numOfDevices, cl_command_queue* commandQueues, cl_kernel kernel) {
	cl_int errNum;
	cl_mem buffers[3];
	// testing code
	CTimer timer;
	int seconds, usecound;
	int array_size = 10;
	float *result = new float[array_size];
	float *a = new float[array_size];
	float *b = new float[array_size];
	for (int i = 0; i < array_size; i++) {
		a[i] = (float)i;
		b[i] = (float)(i * 2);
	}

	buffers[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, array_size * sizeof(float), a, NULL);
	buffers[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, array_size * sizeof(float), b, NULL);
	buffers[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, array_size * sizeof(float), NULL, NULL);


	if (buffers[0] == NULL || buffers[1] == NULL || buffers[2] == NULL) {
		std::cerr << "ERROR creating buffers" << std::endl;

		delete[] b;
		delete[] a;
		delete[] result;

		return false;
	}


	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffers[0]) ||
		clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffers[1]) ||
		clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffers[2]);

	if (!CheckOpenCLError(errNum, "ERROR setting kernel arguments")) {

		delete[] b;
		delete[] a;
		delete[] result;

		return false;
	}

	size_t globalWorkSize[1] = { (size_t)array_size };
	size_t localWorkSize[1] = { 1 };

	for (int i = 0; i < numOfDevices; ++i) {
		errNum = clEnqueueNDRangeKernel(commandQueues[i], kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
		if (!CheckOpenCLError(errNum, "ERROR queueing kernel for execution.")) {

			delete[] b;
			delete[] a;
			delete[] result;

			return false;
		} 

		errNum = clEnqueueReadBuffer(commandQueues[i], buffers[2], CL_TRUE, 0, array_size * sizeof(float), result, 0, NULL, NULL);
		if (!CheckOpenCLError(errNum, "ERROR reading result buffer.")) {

			delete[] b;
			delete[] a;
			delete[] result;

			return false;
		}

	}

	for (int i = 0; i < array_size; i++) {
		std::cout << result[i] << " ";
	}
	std::cout << std::endl << std::endl;
	std::cout << "Executed program successfully." << std::endl;

	delete[] b;
	delete[] a;
	delete[] result;
	return true;
}

bool runOpenCLKernel(cl_context context, cl_kernel kernel) {

	// create memory objects for kernel arguments
	//buffers[0] = clCreateBuffer(context, CL_MEM_READ_ONLY, FLOCK_SIZE * sizeof(cl_uint)


	//clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&input_buffer);
	//clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&output_buffer);

	//cl_int flockSize = FLOCK_SIZE;
	//errNum = clSetKernelArg(kernel, 2, sizeof(flockSize), (void *)&flockSize);


	size_t globalWorkSize[1] = { (size_t)FLOCK_SIZE };
	size_t localWorkSize[1] = { 1 };



	// copy input data
	//errNum = clEnqueueWriteBuffer(
	//	command_queues[0], input_buffer, CL_FALSE, 0,
	//	sizeof(cl_uint)*FLOCK_SIZE, x, 0, NULL, &wrEv[numWrEv++]);

	//// launch the kernel
	//for (int i = 0; i < numOfDevices; ++i) {
	//	errNum = clEnqueueNDRangeKernel(
	//		command_queues[i], 
	//		kernel, 
	//		sizeof(globalWorkSize) / sizeof(size_t), 
	//		globalWorkSize, 
	//		NULL, 
	//		0, 
	//		NULL, 
	//		NULL);
	//}
}

Flocking::Flocking() : _screenWidth(1280),
	_screenHeight(720),
	_time(0.0f),
	_window(nullptr),
	_gameState(GameState::PLAY)
{

	// Create a context and command queue on the device
	context = createOpenlCLContext();
	if (!context) {
		return;
	}

	numOfDevices = countOpenlCLDevices(context);

	// Get all the CPU and GPU devices
	devices = retrieveOpenCLDevices(context, numOfDevices);
	if (!devices) {
		cleanupOpenCLModels(context, numOfDevices, command_queues, program, kernel);
	}

	// Get device information for each device and assign command queues
	command_queues = (cl_command_queue *)malloc(numOfDevices * sizeof(cl_command_queue));

	std::cout << std::endl << "Devices:" << std::endl;
	for (int i = 0; i < numOfDevices; i++) {
		
		if (!printOpenCLDeviceInfo(devices[i])) {
			free(devices);
			cleanupOpenCLModels(context, numOfDevices, command_queues, program, kernel);
		}

		// create command queue for each device
		command_queues[i] = createOpenCLCommandQueue(context, devices[i]);
		if (!command_queues[i]) {
			free(devices);
			cleanupOpenCLModels(context, numOfDevices, command_queues, program, kernel);
			return;
		}

	}
	std::cout << std::endl;


	// create program
	program = createOpenCLProgram(context, numOfDevices, devices, KERNEL_FILE);
	if (!program) {
		cleanupOpenCLModels(context, numOfDevices, command_queues, program, kernel);
		return;
	}

	// create kernel
	kernel = createOpenCLKernel(program, "./flocking.cl");

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
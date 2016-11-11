

// Created by Jarred Jardine & Jack Tsai

#include <stdlib.h>
#include <string>
#include <iostream>
#include "Flocking.h"
#include "Errors.h"
#include "OpenCLHelper.h"



bool Flocking::runOpenCLKernel(cl_context context, cl_uint numOfDevices, cl_command_queue* commandQueues, cl_kernel kernel, float time) {

	cl_int errNum;
	int numWrEv = 0;
	cl_event wrEv[1];
	int numRdEv = 0;
	cl_event rdEv[MAX_NUM_DEV];
	cl_event enqEv[MAX_NUM_DEV];
	// timer setup
	std::clock_t start;
	double duration;
	start = std::clock();

	std::vector<Boid> result;

	input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numBoids * sizeof(Boid), &_boids[0], NULL);
	output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, numBoids * sizeof(Boid), NULL, NULL);
	errNum = clEnqueueWriteBuffer(commandQueues[0], input_buffer, CL_FALSE, 0, numBoids * sizeof(Boid), &_boids[0], 0, NULL, &wrEv[numWrEv++]);
	if (!CheckOpenCLError(errNum, "ERROR writing input buffer.")) {
		return false;
	}

	if (input_buffer == NULL || output_buffer == NULL) {
		std::cerr << "ERROR creating buffers" << std::endl;
		return false;
	}


	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer)
		|| clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer)
		|| clSetKernelArg(kernel, 2, sizeof(int), &numBoids)
		|| clSetKernelArg(kernel, 3, sizeof(float), &time)
		|| clSetKernelArg(kernel, 4, sizeof(Boid), &_boidLeader)
		|| clSetKernelArg(kernel, 5, sizeof(int), &_screenWidth)
		|| clSetKernelArg(kernel, 6, sizeof(int), &_screenHeight);

	if (!CheckOpenCLError(errNum, "ERROR setting kernel arguments")) {
		return false;
	}

	size_t globalWorkSize[1] = { (size_t)numBoids };
	size_t localWorkSize[1] = { 1 };

	for (int i = 0; i < numOfDevices; ++i) {
		errNum = clEnqueueNDRangeKernel(commandQueues[i], kernel, 1, NULL, globalWorkSize, NULL, numWrEv, wrEv, &enqEv[i]);
		if (!CheckOpenCLError(errNum, "ERROR queueing kernel for execution.")) {
			return false;
		}

		errNum = clEnqueueReadBuffer(commandQueues[i], output_buffer, CL_TRUE, 0, numBoids * sizeof(Boid), &_boids[0], 1, &enqEv[i], &rdEv[numRdEv++]);
		if (!CheckOpenCLError(errNum, "ERROR reading result buffer.")) {
			return false;
		}

	}

	return true;

}

Flocking* _instance = 0;
Flocking* Flocking::getInstance()
{
	if (!_instance) {
		_instance = new Flocking;
	}
	
	return _instance;
}

Flocking::Flocking() : _screenWidth(1280),
	_screenHeight(720),
	_time(0.0f),
	_window(nullptr),
	_gameState(GameState::PLAY)
{


	// timer setup
	_currentTime = std::clock();

	initSystems();

	initShaders();

	setupFlock();

	gameLoop();
	
}


Flocking::~Flocking()
{
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

}

void Flocking::initShaders()
{
	_colorProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
	_colorProgram.addAttribute("vertexPosition");
	_colorProgram.addAttribute("vertexColor");
	_colorProgram.linkShaders();
}

void Flocking::initOpenCL(cl_device_type deviceType = CL_DEVICE_TYPE_ALL)
{
	if (numOfDevices)
		cleanupOpenCLModels(context, &numOfDevices, command_queues, program, kernel);

	// Create a context and command queue on the device
	context = createOpenCLContext(deviceType);
	if (!context) {
		return;
	}

	numOfDevices = countOpenlCLDevices(context);

	// Get all the CPU and GPU devices
	devices = retrieveOpenCLDevices(context, numOfDevices);
	if (!devices) {
		cleanupOpenCLModels(context, &numOfDevices, command_queues, program, kernel);
	}

	// Get device information for each device and assign command queues
	command_queues = (cl_command_queue *)malloc(numOfDevices * sizeof(cl_command_queue));

	std::cout << std::endl << "Devices:" << std::endl;
	for (int i = 0; i < numOfDevices; i++) {

		if (!printOpenCLDeviceInfo(devices[i])) {
			free(devices);
			cleanupOpenCLModels(context, &numOfDevices, command_queues, program, kernel);
		}

		// create command queue for each device
		command_queues[i] = createOpenCLCommandQueue(context, devices[i]);
		if (!command_queues[i]) {
			free(devices);
			cleanupOpenCLModels(context, &numOfDevices, command_queues, program, kernel);
			return;
		}

	}
	std::cout << std::endl;


	// create program
	program = createOpenCLProgram(context, numOfDevices, devices, KERNEL_FILE);
	if (!program) {
		cleanupOpenCLModels(context, &numOfDevices, command_queues, program, kernel);
		return;
	}

	// create kernel
	kernel = createOpenCLKernel(program, KERNEL_FUNCTION);
	if (!kernel) {
		cleanupOpenCLModels(context, &numOfDevices, command_queues, program, kernel);
		return;
	}
}

void Flocking::gameLoop()
{
	runState = RunState::SERIAL; // starts in serial mode
	while (_gameState != GameState::EXIT)
	{
		std::clock_t newTime = std::clock();
		float timeSinceLastFrame = (newTime - _currentTime) / (float)CLOCKS_PER_SEC;
		_currentTime = newTime;

		processInput();
		
		update(timeSinceLastFrame);

		render();
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
		case SDL_KEYDOWN:
			switch (evnt.key.keysym.sym)
			{
			case SDLK_1:
				printf("Serial\n");
				runState = RunState::SERIAL;
				break;
			case SDLK_2:
				printf("CPU\n");
				runState = RunState::CPU;
				initOpenCL(CL_DEVICE_TYPE_CPU);
				break;
			case SDLK_3:
				runState = RunState::GPU;
				initOpenCL(CL_DEVICE_TYPE_GPU);
				break;
			case SDLK_4:
				runState = RunState::CPU_GPU;
				initOpenCL(CL_DEVICE_TYPE_ALL);
				break;
			case SDLK_q:
				//increase boid count
				if (numBoids < MAXBOIDS)
					numBoids += INCREASE_BOID_AMOUNT;
				printf("Boid count: %i\n", numBoids);
				break;
			case SDLK_a:
				// decrease boid count
				if (numBoids > MINBOIDS)
					numBoids -= INCREASE_BOID_AMOUNT;
				printf("Boid count: %i\n", numBoids);
				break;
			}
			break;
		}
	}
}

void Flocking::setupFlock()
{
	_boidLeader = Boid(_screenWidth / 2, _screenHeight / 2);
	for (int i = 0; i < numBoids; i++)
	{
		_boids.push_back(Boid(_screenWidth / 2 + rand() % 100 - 50, _screenHeight / 2 + rand() % 100 - 50));
	}
}

void Flocking::checkFlockNums()
{

	if (_boids.size() < numBoids)
	{
		int numToAdd = numBoids - _boids.size();
		for (int i = numToAdd; i > 0; i--)
		{
			_boids.push_back(Boid(_screenWidth / 2 + rand() % 100 - 50, _screenHeight / 2 + rand() % 100 - 50));
		}
		printf("The real boid count after updating: %i\n", _boids.size());
	}

	if (_boids.size() > numBoids)
	{
		int numToRemove = _boids.size() - numBoids;
		for (int i = numToRemove; i > 0; i--)
		{
			_boids.pop_back();
		}
		printf("The real boid count after updating: %i\n", _boids.size());
	}
}

void Flocking::update(float timeSinceLastFrame)
{
	_time += 0.01;

	_boidLeader.Update(timeSinceLastFrame, _screenWidth, _screenHeight);

	checkFlockNums();

	if (runState == SERIAL)
	{
		for (std::vector<Boid>::iterator it = _boids.begin(); it != _boids.end(); it++)
		{
			(*it).Flock(_boids, _boidLeader, timeSinceLastFrame);
			(*it).Update(timeSinceLastFrame, _screenWidth, _screenHeight);
		}
	}
	else
	{
		if (context && numOfDevices && command_queues && kernel)
			runOpenCLKernel(context, numOfDevices, command_queues, kernel, timeSinceLastFrame);
	}
}

void Flocking::render()
{
	// Set the base depth to 1.0
	glClearDepth(1.0);
	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_colorProgram.use();

	GLuint timeLocation = _colorProgram.getUniformLocation("time");
	glUniform1f(timeLocation, _time);

	// for each boid in list of boids render
	renderBoid(_boidLeader);
	
	for (std::vector<Boid>::iterator it = _boids.begin(); it != _boids.end(); it++)
	{
		renderBoid(*it);
	}


	_colorProgram.unuse();

	glEnd();

	// Swap buffer and draw everything to the screen.
	SDL_GL_SwapWindow(_window);
}

void Flocking::renderBoid(Boid boid)
{
	Vertex vertexData[3];
	vertexData[0].position.x = boid.position.x / (1280 / 2) - 1;
	vertexData[0].position.y = boid.position.y / (720 / 2) - 1 + 0.03;

	vertexData[1].position.x = boid.position.x / (1280 / 2) - 1 + 0.015;
	vertexData[1].position.y = boid.position.y / (720 / 2) - 1 - 0.03;

	vertexData[2].position.x = boid.position.x / (1280 / 2) - 1 - 0.015;
	vertexData[2].position.y = boid.position.y / (720 / 2) - 1 - 0.03;

	glGenBuffers(1, &_boidVbo);

	glBindBuffer(GL_ARRAY_BUFFER, _boidVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	// Tell opengl that we want to use the first
	// attribute array. We only need one array right
	// now since we are only using positon.
	glEnableVertexAttribArray(0);

	// This is the position attribute pointer
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	// This is the color attribute pointer
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(0);
}

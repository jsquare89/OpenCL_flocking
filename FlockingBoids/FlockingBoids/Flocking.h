#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <CL/cl.h>


#include "GLSLProgram.h"
#include "Flock.h"

#define F (8)
#define MAX_NUM_DEV (4)

enum class GameState { PLAY, EXIT };

class Flocking
{
public:
	Flocking();
	~Flocking();

	void run();



	//OpenCL
	cl_program program;
	cl_kernel kernel;
	cl_context context;
	cl_command_queue *command_queues;
	cl_device_id *devices;
	cl_uint numOfDevices;
	cl_mem buffers[3];
	cl_mem input_buffers;
	cl_mem output_buffer;

private:
	void initSystems();
	void initShaders();
	void gameLoop();
	void processInput();
	void Render();

	void SetupFlock();

	SDL_Window* _window;
	int _screenWidth;
	int _screenHeight;
	GameState _gameState;

	GLSLProgram _colorProgram;

	int _flockNums;
	Flock* _pflock;

	float _time;
};


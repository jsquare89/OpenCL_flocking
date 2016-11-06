#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>


#include "GLSLProgram.h"
#include "Flock.h"

#define F[8]
#define MAX_NUM_DEV[4]

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
	cl_command_queue command_queue[4];
	cl_device_id device[4];
	int load[4];
	cl_uint noOfDevices;
	cl_mem buffer;

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


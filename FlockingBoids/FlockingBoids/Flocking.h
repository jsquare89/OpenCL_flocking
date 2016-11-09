#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <CL/cl.h>

#include <vector>

#include "GLSLProgram.h"
#include "Boid.h"

#define F (8)
#define MAX_NUM_DEV (4)

#define FLOCK_SIZE (1)

enum class GameState { PLAY, EXIT };

class Flocking
{
public:	
	static Flocking *getInstance();

private:
	Flocking();
	~Flocking();

	void initSystems();
	void initShaders();
	void initOpenCL();
	void gameLoop();
	void processInput();
	void update();
	void render();
	void renderBoid(Boid boid);

	void setupFlock();
	Boid wrapBorder(Boid boid);

	// SDL
	SDL_Window* _window;
	int _screenWidth;
	int _screenHeight;
	GameState _gameState;

	// OpenGL
	GLSLProgram _colorProgram;
	GLuint _boidVbo;
	float _time;

	// OpenCL
	cl_program program;
	cl_kernel kernel;
	cl_context context;
	cl_command_queue *command_queues;
	cl_device_id *devices;
	cl_uint numOfDevices;
	cl_mem buffers[3];
	cl_mem input_buffer;
	cl_mem output_buffer;
	bool runOpenCLKernel(cl_context context, cl_uint numOfDevices, cl_command_queue* commandQueues, cl_kernel kernel);

	Boid _boidLeader;
	std::vector<Boid> _boids;
	const int numBoids = FLOCK_SIZE;
};


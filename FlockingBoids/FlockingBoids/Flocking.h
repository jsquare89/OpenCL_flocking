#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <CL/cl.h>

#include <vector>
#include <ctime>

#include "GLSLProgram.h"
#include "Boid.h"

#define F (8)
#define MAX_NUM_DEV (1)

#define FLOCK_SIZE (30)
#define MINBOIDS (10)
#define MAXBOIDS (1000)
#define INCREASE_BOID_AMOUNT (10)

#define MAX_SPEED (25)
#define SEPARATION_RADIUS (30)
#define ARRIVE_TARGET_RADIUS (30)
#define ARRIVE_SLOW_RADIUS (75)
#define ARRIVE_TIME_TO_TARGET (0.1f)
#define SEPARATION_WEIGHT (2)
#define ARRIVAL_WEIGHT (1)

enum class GameState { PLAY, EXIT };
enum RunState { SERIAL, GPU, CPU, CPU_GPU };

class Flocking
{
public:	
	static Flocking *getInstance();

private:
	Flocking();
	~Flocking();
	std::clock_t _currentTime;

	void initSystems();
	void initShaders();
	void initOpenCL(cl_device_type deviceType);
	void gameLoop();
	void processInput();
	void update(float timeSinceLastFrame);
	PVector alignment(Boid boid);
	PVector cohesion(Boid boid);
	PVector separation(Boid boid);
	void render();
	void renderBoid(Boid boid);

	void setupFlock();
	void checkFlockNums();
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
	cl_context gpu_context;
	cl_context cpu_context;
	cl_command_queue *command_queues;
	cl_device_id *devices;
	cl_uint numOfDevices;
	cl_mem buffers[3];
	cl_mem input_buffer;
	cl_mem output_buffer;
	bool runOpenCLKernel(cl_context context, cl_uint numOfDevices, cl_command_queue* commandQueues, cl_kernel kernel, float time);

	Boid _boidLeader;
	std::vector<Boid> _boids;
	int numBoids = FLOCK_SIZE;
	RunState runState;
};


#pragma once

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <GL/glew.h>
#include <CL/cl.h>

#include <vector>
#include <ctime>

#include "GLSLProgram.h"
#include "Boid.h"

#define MAX_NUM_DEV (1)

#define FLOCK_SIZE (30)
#define MINBOIDS (10)
#define MAXBOIDS (2000)
#define INCREASE_BOID_AMOUNT (10)

#define MAX_SPEED (50)
#define WANDER_CIRCLE_DISTANCE (60)
#define WANDER_CIRCLE_RADIUS (30)
#define WANDER_ANGLE_CHANGE (0.1)
#define SEPARATION_RADIUS (50)
#define ARRIVE_TARGET_RADIUS (30)
#define ARRIVE_SLOW_RADIUS (75)
#define ARRIVE_TIME_TO_TARGET (0.1f)
#define SEPARATION_WEIGHT (3)
#define ARRIVAL_WEIGHT (1)

#define KERNEL_FILE ("./flocking.cl")
#define KERNEL_FUNCTION ("update")

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
	void fpsCounter();
	void processInput();
	void update(float timeSinceLastFrame);
	void render();
	void renderLeader();
	void renderBoid(Boid boid);

	void setupFlock();
	void checkFlockNums();
	// FPS counter
	Uint32 startclock = 0;
	Uint32 deltaclock = 0;
	Uint32 currentFPS = 0;

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
	cl_mem input_buffer;
	cl_mem output_buffer;
	bool runOpenCLKernel(cl_context context, cl_uint numOfDevices, cl_command_queue* commandQueues, cl_kernel kernel, float time);

	Boid _boidLeader;
	std::vector<Boid> _boids;
	int numBoids = FLOCK_SIZE;
	RunState runState;
};


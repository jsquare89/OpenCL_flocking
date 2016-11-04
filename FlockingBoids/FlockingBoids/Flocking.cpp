// Created by Jarred Jardine

#include "Flocking.h"
#include "Errors.h"

#include <iostream>
#include <string>


Flocking::Flocking() : _screenWidth(1240),
_screenHeight(720),
_time(0.0f),
_window(nullptr),
_gameState(GameState::PLAY)
{

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
			std::cout << evnt.motion.x << " " << evnt.motion.y << std::endl;
			break;
		}
	}
}

void Flocking::SetupFlock()
{
	_flockNums = 300;
	for (int i = 0; i < _flockNums; i++)
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
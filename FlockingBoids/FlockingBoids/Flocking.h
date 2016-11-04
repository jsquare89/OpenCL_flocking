#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>


#include "GLSLProgram.h"
#include "Flock.h"

enum class GameState { PLAY, EXIT };

class Flocking
{
public:
	Flocking();
	~Flocking();

	void run();

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


// Created by Jarred Jardine
#pragma once

#define _USE_MATH_DEFINES
#include "PVector.h"
#include <math.h>
#include <cstdlib>
#include "Vertex.h"

class Boid
{
public:
	Boid();
	~Boid();

	void Setup(float x, float y);
	void Render();
	void Run();
private:
	void Update();

	PVector _position;
	PVector _velocity;
	PVector _acceleration;
	float _r;
	float _maxForce;
	float _maxSpeed;
	GLuint _vboID;
	Vertex _vertexData[3];
};


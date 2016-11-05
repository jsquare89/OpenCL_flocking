// Created by Jarred Jardine
#pragma once

#define _USE_MATH_DEFINES
#include "PVector.h"
#include <math.h>
#include <cstdlib>
#include "Vertex.h"
#include <vector>

class Boid
{
public:
	Boid();
	~Boid();

	void Setup(float x, float y);
	void Render();
	void Run(std::vector<Boid> &boids);
	void Flock(std::vector<Boid> &boids);
	void Borders();
	void ApplyForce(PVector force);

	PVector Seek(PVector target);
	PVector Seperation(std::vector<Boid> &boids);
	PVector Align(std::vector<Boid> &boids);
	PVector Cohesion(std::vector<Boid> &boids);
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
	float _width;
	float _height;
};


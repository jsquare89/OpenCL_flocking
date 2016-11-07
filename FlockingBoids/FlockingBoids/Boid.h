// Created by Jarred Jardine
#pragma once

#define _USE_MATH_DEFINES
#include "PVector.h"
#include <math.h>
#include <cstdlib>
#include "Vertex.h"
#include <vector>

struct Boid {
	PVector _position;
	PVector _velocity;
	PVector _acceleration;

	Boid();
	~Boid();

	void Setup(float x, float y);
	void Render();
	void Run(std::vector<Boid> &boids, Boid &leader);
	void Flock(std::vector<Boid> &boids, Boid &leader);
	void Update();
	void Borders();
	void ApplyForce(PVector force);

	void Wander();
	PVector Seek(PVector target);
	PVector Seperation(std::vector<Boid> &boids);
	PVector Align(std::vector<Boid> &boids);
	PVector Cohesion(std::vector<Boid> &boids);

	float _r;
	float _maxForce;
	float _maxSpeed;
	GLuint _vboID;
	Vertex _vertexData[3];
	float _width;
	float _height;
};


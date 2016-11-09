// Created by Jarred Jardine
#pragma once

#define _USE_MATH_DEFINES
#include "PVector.h"
#include <math.h>
#include <cstdlib>
#include "Vertex.h"
#include <vector>

struct Boid {
	PVector position;
	PVector velocity;
	PVector acceleration;

	Boid();
	Boid(float x, float y);
	void Run(std::vector<Boid> boids, Boid leader);
	void Flock(std::vector<Boid> boids, Boid leader);
	void Update();
	void ApplyForce(PVector force);

	void Wander();
	PVector Seek(PVector target);
	PVector Seperation(std::vector<Boid> &boids);
	PVector Align(std::vector<Boid> &boids);
	PVector Cohesion(std::vector<Boid> &boids);

	float maxForce;
	float maxSpeed;
};


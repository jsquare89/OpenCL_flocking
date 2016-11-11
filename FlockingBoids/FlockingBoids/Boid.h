// Created by Jarred Jardine
#pragma once

#define _USE_MATH_DEFINES
#include "PVector.h"
#include <math.h>
#include <cstdlib>
#include "Vertex.h"
#include <vector>

struct Boid {
	PVector position, velocity, acceleration;
	float rotation, _temp; // weird that OpenCL don't like me defining only one float

	Boid();
	Boid(float x, float y);
	void Flock(std::vector<Boid> boids, Boid leader, float time);
	void Update(float time, int width, int height);
	PVector Wander();
	PVector Arrive(PVector target);
	PVector Separation(std::vector<Boid> &boids);

};


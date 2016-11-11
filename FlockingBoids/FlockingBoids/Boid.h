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
	void Flock(std::vector<Boid> boids, Boid leader, float time);
	void Update(float time);

	void Wander(float time);
	PVector Arrive(PVector target);
	PVector Separation(std::vector<Boid> &boids);

};


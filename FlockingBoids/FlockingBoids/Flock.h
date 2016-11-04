#pragma once
#include "Boid.h"
#include <vector>.
#include <iterator>

class Flock
{
public:
	Flock();
	~Flock();

	void Run();
	void AddBoids(Boid b);
	void Render();
	
private:
	Boid _boid;
	std::vector<Boid> _boids;

};


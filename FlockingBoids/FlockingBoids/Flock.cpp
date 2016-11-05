#include "Flock.h"
#include <iostream>


Flock::Flock()

{
	_boid.Setup(0, 0);
}


Flock::~Flock()
{
}


void Flock::Run() 
{
	for (std::vector<Boid>::iterator it = _boids.begin(); it != _boids.end(); it++)
	{
		it->Run(_boids);
	}
}
void Flock::AddBoids(Boid b)
{
	_boids.push_back(b);
	std::cout << "Boid Vector count: " << _boids.size() << std::endl;
}

void Flock::Render()
{
	// for each boid in list of boids render
	//_boid.Render();
	//std::cout << "Boid Vector size at Render: " << _boids.size() << std::endl;
	for (std::vector<Boid>::iterator it = _boids.begin(); it != _boids.end(); it++)
	{
		it->Render();
	}
}

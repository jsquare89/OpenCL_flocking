#include "Flock.h"
#include <iostream>


Flock::Flock()

{
	_boidLeader.Setup(900, 500);
	//_boidLeader.ApplyForce(PVector{ 1,0 });

	
}


Flock::~Flock()
{
}


void Flock::Run() 
{
	_boidLeader.Wander();
	for (std::vector<Boid>::iterator it = _boids.begin(); it != _boids.end(); it++)
	{
		it->Flock(_boids, _boidLeader);
	}
	for (std::vector<Boid>::iterator it = _boids.begin(); it != _boids.end(); it++)
	{
		it->Update();
	}
}
void Flock::AddBoids(Boid b)
{
	_boids.push_back(b);
}

void Flock::Render()
{
	// for each boid in list of boids render
	_boidLeader.Render();
	//std::cout << "Boid Vector size at Render: " << _boids.size() << std::endl;
	for (std::vector<Boid>::iterator it = _boids.begin(); it != _boids.end(); it++)
	{
		it->Render();
	}
}

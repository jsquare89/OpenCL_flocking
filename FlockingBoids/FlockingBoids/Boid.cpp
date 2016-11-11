#include "Boid.h"



Boid::Boid()
{
	Boid(0, 0);
}

Boid::Boid(float x, float y)
{
	acceleration = PVector{ 0,0 };
	float angle = (float)(rand() % 360 + 1)*(M_PI / 180);
	velocity = PVector{ (float)cos(angle),(float)sin(angle) };
	position = PVector{ x, y };
	maxSpeed = 25.0;
	maxForce = 0.9;
}

void Boid::Run(std::vector<Boid> boids, Boid leader)
{
	//Flock(boids, leader);
	//Update();
}

void Boid::Flock(std::vector<Boid> boids, Boid leader, float time)
{
	// get new acceleration based on 3 rules


	//PVector seek = Seek(leader.position);
	PVector arrive = Arrive(leader.position);
	PVector sep = Separation(boids);
	//PVector ali = Align(boids);
	//PVector coh = Cohesion(boids);

	arrive.mult(1);
	sep.mult(2);
	//ali.mult(1);
	//coh.mult(1);
	ApplyForce(arrive);
	ApplyForce(sep);
	//ApplyForce(ali);
	//ApplyForce(coh);
}

void Boid::Update(float time)
{
	velocity.add(acceleration);
	//velocity.limit(maxSpeed);
	velocity.normalize();
	velocity.mult(maxSpeed);

	PVector velocityCopy = velocity;
	velocityCopy.mult(time);
	position.add(velocityCopy);
	acceleration.mult(0); // Reset acceleration
}

void Boid::ApplyForce(PVector force)
{
	acceleration.add(force);
}

void Boid::Wander(float time)
{
	Update(time);
}

PVector Boid::Arrive(PVector target)
{
	float targetRadius = 30.f;
	float timeToTarget = 0.1f;
	float targetSpeed = 0.f;
	float slowRadius = 75.f;

	PVector direction = PVector().sub(target, position);
	float dist = direction.mag();

	if (dist < targetRadius)
		return PVector{ 0,0 };
	if (dist > targetRadius)
		targetSpeed = maxSpeed;
	else
		targetSpeed = maxSpeed*dist / slowRadius;

	PVector targetVelocity = direction;
	targetVelocity.normalize();
	targetVelocity.mult(targetSpeed);

	PVector steer = PVector().sub(targetVelocity, velocity);
	steer.div(timeToTarget);

	if (steer.mag() > maxForce)
	{
		steer.normalize();
		steer.mult(maxSpeed);
	}

	return steer;
}

PVector Boid::Separation(std::vector<Boid> &boids)
{
	float separationRadius = 30.0f;
	PVector steer{ 0,0 };
	int neighbourCount = 0;
	// check each boid in ssystem, check if its close
	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
	{
		// get distance of neighbor
		float d = PVector().dist(position, neighborItr->position);
		if ((d > 0) && (d <= separationRadius))
		{
			// Calculate vector point away from neighbor
			steer.x += neighborItr->position.x - position.x;
			steer.y += neighborItr->position.y - position.y;
			neighbourCount++; // Keep track of how many
		}
	}
	// Average -- dive by amount
	if (neighbourCount > 0)
	{
		steer.div((float)neighbourCount);
		steer.mult(-1);
	}

	if (steer.mag() > 0)
	{
		steer.normalize();
		steer.mult(maxSpeed);
		steer.limit(maxForce);
	}
	return steer;
}

//
//PVector Boid::Seek(PVector target)
//{
//	PVector desired = PVector().sub(target, position);
//	desired.normalize();
//	desired.mult(maxSpeed);
//
//	PVector steer = PVector().sub(desired, velocity);
//	steer.limit(maxForce);
//	return steer;	
//}
//
//PVector Boid::Seperation(std::vector<Boid> &boids)
//{
//	float desiredSep = 40.0f;
//	PVector steer{ 0,0 };
//	int count = 0;
//	// check each boid in ssystem, check if its close
//	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
//	{
//		// get distance of neighbor
//		float d = PVector().dist(position, neighborItr->position);
//		if ((d > 0) && (d < desiredSep))
//		{
//			// Calculate vector point away from neighbor
//			PVector diff = PVector().sub(position, neighborItr->position);
//			diff.normalize();
//			diff.div(d); // weight by diff
//			steer.add(diff);
//			count++; // Keep track of how many
//		}
//		// Average -- dive by amount
//		if (count > 0)
//		{
//			steer.div((float)count);
//		}
//
//		if (steer.mag() > 0)
//		{
//			steer.normalize();
//			steer.mult(maxSpeed);
//			steer.sub(velocity);
//			steer.limit(maxForce);
//		}
//		return steer;
//	}
//}
//
//PVector Boid::Align(std::vector<Boid> &boids)
//{
//	float neighborDist = 10.f;
//
//	PVector result = PVector{ 0, 0 };
//	int neighborCount = 0;
//	for (std::vector<Boid>::iterator it = boids.begin(); it != boids.end(); ++it)
//	{
//		Boid other = (*it);
//		if (this != &(*it)) {
//			this->position.sub((*it))
//			if ( (this->position.sub((*it).position))->mag() < neighborDist )
//		}
//		float d = PVector().dist(position, it->position);
//		if ((d > 0) && (d < neighborDist))
//		{
//			sum.add(it->velocity);
//			count++;
//		}
//	}
//
//
//	
//	PVector sum = PVector{ 0, 0 };
//	int count = 0;
//	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
//	{
//		float d = PVector().dist(position, neighborItr->position);
//		if ((d > 0) && (d < neighborDist)) 
//		{
//			sum.add(neighborItr->velocity);
//			count++;
//		}
//	}
//	if (count > 0) {
//		sum.div((float)count);
//		// First two lines of code below could be condensed with new PVector setMag() method
//		// Not using this method until Processing.js catches up
//		// sum.setMag(maxspeed);
//
//		// Implement Reynolds: Steering = Desired - Velocity
//		sum.normalize();
//		sum.mult(maxSpeed);
//		PVector steer = PVector().sub(sum, velocity);
//		steer.limit(maxForce);
//		return steer;
//	}
//	else {
//		return PVector{ 0, 0 };
//	}
//}
//
//PVector Boid::Cohesion(std::vector<Boid> &boids)
//{
//	float neighborDist = 100.f;
//	PVector sum = PVector{ 0, 0 };
//	int count = 0;
//	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
//	{
//		float d = PVector().dist(position, neighborItr->position);
//		if ((d > 0) && (d < neighborDist))
//		{
//			sum.add(neighborItr->position);
//			count++;
//		}
//	}
//	if (count > 0)
//	{
//		sum.div(count);
//		return Seek(sum);
//	}
//	else
//	{
//		return PVector{ 0,0 };
//	}
//}

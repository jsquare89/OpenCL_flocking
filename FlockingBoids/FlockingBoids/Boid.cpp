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
	maxSpeed = 2.0;
	maxForce = 0.8;
}

void Boid::Run(std::vector<Boid> boids, Boid leader)
{
	Flock(boids, leader);
	Update();
}

void Boid::Flock(std::vector<Boid> boids, Boid leader)
{
	// get new acceleration based on 3 rules
	
	
	//PVector seek = Seek(leader.position);
	PVector arrive = Arrive(leader.position);
	PVector sep = Separation(boids);
	//PVector ali = Align(boids);
	//PVector coh = Cohesion(boids);

	sep.mult(1);
	//ali.mult(1);
	//coh.mult(1);
	ApplyForce(arrive);
	ApplyForce(sep);
	//ApplyForce(ali);
	//ApplyForce(coh);

}

void Boid::Update()
{
	velocity.add(acceleration);
	velocity.limit(maxSpeed);
	position.add(velocity);
	acceleration.mult(0); // Reset acceleration
}

void Boid::ApplyForce(PVector force)
{
	acceleration.add(force);
}

void Boid::Wander()
{
	

	Update();
}

//PVector Boid::Wanderer()
//{
//	float wanderRate = 2.f; //? not sure what this should be yet
//	float wanderOffset = 2.f; // not sure what this should be yet
//	float wanderRadius = 40.f;
//
//
//	//1. calculate the target to dleegate to face
//	// update wander orientation
//	wanderOrientation += randomBinomial() * wanderRate;
//
//	// Calculate the combined target orientation
//	float targetOrientation = wanderOrientation + orientation;
//
//	// Calculate the center of the wander cicle
//	PVector target = PVector().angleToVector(orientation);
//	target.add(wanderOffset);
//	target = PVector{ position.x + target.x, position.y + target.y };
//		
//	
//	//Calculate the target location
//	PVector targetOrientationVector = PVector().angleToVector(targetOrientation);
//	targetOrientationVector.mult(wanderRadius);
//	target.add(targetOrientationVector);
//
//	//2. delegate to face
//	PVector steer = Face(target);
//
//	//3. now set the linear acceleration to be at full acceleration in the direction of the orientation
//	steer = PVector().angleToVector(orientation); steer.mult(maxSpeed);
//
//	return steer;
//}
//
//
//PVector Boid::Face(PVector target)
//{
//	//1. calculate the target to delegate to align
//	// work out the direction to target
//	PVector dir = PVector().sub(target, position);
//
//	// check for a zero direction, and make no change if so
//	if (dir.mag() == 0)
//		return target;
//
//	// put the target together
//
//	
//	//2. Delegate to align
//	//return Align(target);
//
//	//Align.target = explicitTarget
//	//	21 Align.target.orientation = atan2(-direction.x, direction.z)
//	//	22
//	//	23 # 2. Delegate to align
//	//	24 return Align.getSteering()
//
//}


PVector Boid::Seek(PVector target)
{
	PVector desired = PVector().sub(target, position);
	desired.normalize();
	desired.mult(maxSpeed);

	PVector steer = PVector().sub(desired, velocity);
	steer.limit(maxForce);
	return steer;	
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

PVector Boid::Align(std::vector<Boid> &boids)
{
	float neighborDist = 10.f;
	PVector sum = PVector{ 0, 0 };
	int count = 0;
	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
	{
		float d = PVector().dist(position, neighborItr->position);
		if ((d > 0) && (d < neighborDist)) 
		{
			sum.add(neighborItr->velocity);
			count++;
		}
	}
	if (count > 0) {
		sum.div((float)count);
		// First two lines of code below could be condensed with new PVector setMag() method
		// Not using this method until Processing.js catches up
		// sum.setMag(maxspeed);

		// Implement Reynolds: Steering = Desired - Velocity
		sum.normalize();
		sum.mult(maxSpeed);
		PVector steer = PVector().sub(sum, velocity);
		steer.limit(maxForce);
		return steer;
	}
	else {
		return PVector{ 0, 0 };
	}
}

PVector Boid::Cohesion(std::vector<Boid> &boids)
{
	float neighborDist = 100.f;
	PVector sum = PVector{ 0, 0 };
	int count = 0;
	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
	{
		float d = PVector().dist(position, neighborItr->position);
		if ((d > 0) && (d < neighborDist))
		{
			sum.add(neighborItr->position);
			count++;
		}
	}
	if (count > 0)
	{
		sum.div(count);
		return Seek(sum);
	}
	else
	{
		return PVector{ 0,0 };
	}
}

float randomBinomial()
{
	return rand() - rand();
}

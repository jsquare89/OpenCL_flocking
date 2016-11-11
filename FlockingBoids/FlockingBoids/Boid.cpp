#include "Boid.h"
#include "Flocking.h"

Boid::Boid()
{
	Boid(0, 0);
}

Boid::Boid(float x, float y)
{
	acceleration = PVector{ 0,0 };
	float angle = (float)(rand() % 360 + 1)*(M_PI / 180);
	velocity = PVector{ (float)cos(angle),(float)sin(angle) };
	velocity.normalize();
	velocity.mult(MAX_SPEED);
	position = PVector{ x, y };
}

void Boid::Flock(std::vector<Boid> boids, Boid leader, float time)
{
	PVector arrive = Arrive(leader.position);
	PVector sep = Separation(boids);

	arrive.mult(ARRIVAL_WEIGHT);
	sep.mult(SEPARATION_WEIGHT);
	acceleration.add(arrive);
	acceleration.add(sep);
}

void Boid::Update(float time, int width, int height)
{
	velocity.add(acceleration);
	if (velocity.mag() > MAX_SPEED) {
		velocity.normalize();
		velocity.mult(MAX_SPEED);
	}

	PVector velocityCopy = velocity;
	velocityCopy.mult(time);
	position.add(velocityCopy);
	acceleration.mult(0); // Reset acceleration

	// wrap border
	if (position.x < 0)
		position.x += width;
	else if (position.y < 0)
		position.y += width;
	else if (position.x > height)
		position.x -= height;
	else if (position.y > height)
		position.y -= height;
}

PVector Boid::Arrive(PVector target)
{
	float targetSpeed = 0.f;

	PVector direction = PVector().sub(target, position);
	float dist = direction.mag();

	if (dist < ARRIVE_TARGET_RADIUS)
		return PVector{ 0,0 };
	if (dist > ARRIVE_TARGET_RADIUS)
		targetSpeed = MAX_SPEED;
	else
		targetSpeed = MAX_SPEED * dist / ARRIVE_SLOW_RADIUS;

	PVector targetVelocity = direction;
	targetVelocity.normalize();
	targetVelocity.mult(targetSpeed);

	PVector steer = PVector().sub(targetVelocity, velocity);
	steer.div(ARRIVE_TIME_TO_TARGET);

	if (steer.mag() > MAX_SPEED)
	{
		steer.normalize();
		steer.mult(MAX_SPEED);
	}

	return steer;
}

PVector Boid::Separation(std::vector<Boid> &boids)
{
	PVector steer{ 0,0 };
	int neighbourCount = 0;
	// check each boid in ssystem, check if its close
	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
	{
		// get distance of neighbor
		float d = PVector().dist(position, neighborItr->position);
		if ((d > 0) && (d <= SEPARATION_RADIUS))
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

	if (steer.mag() > MAX_SPEED)
	{
		steer.normalize();
		steer.mult(MAX_SPEED);
	}
	return steer;
}
#include "Boid.h"



Boid::Boid()
{
	Setup(640, 360);
}

void Boid::Setup(float x, float y)
{
	_acceleration = PVector{ 0,0 };
	float angle = (float)(rand() % 360 + 1)*(M_PI / 180);
	_velocity = PVector{ (float)cos(angle),(float)sin(angle) };
	_position = PVector{ x, y };
	_r = 1;
	_maxSpeed = 1.0;
	_maxForce = 0.9;
	_width = 1;
	_height = 1;

	_vboID = 0;
	if (_vboID == 0)
	{
		glGenBuffers(1, &_vboID);
	}

	

	// Create boid triagle
	_vertexData[0].position.x = x;
	_vertexData[0].position.y = y +0.03;

	_vertexData[1].position.x = x +0.015;
	_vertexData[1].position.y = y -0.03;

	_vertexData[2].position.x = x -0.015;
	_vertexData[2].position.y = y -0.03;

	// Color vertices
	for (int i = 0; i < 3; i++)
	{
		_vertexData[i].color.r = 255;
		_vertexData[i].color.g = 0;
		_vertexData[i].color.b = 255;
		_vertexData[i].color.a = 255;
	}

	// to do! rotate boid in the direction of the velocity. or change the boids into circles.


}

Boid::~Boid()
{
	if (_vboID != 0)
	{
		glDeleteBuffers(1, &_vboID);
	}
}

void Boid::Run(std::vector<Boid> &boids, Boid &leader)
{
	Flock(boids, leader);
	Update();
	Borders();
}

void Boid::Flock(std::vector<Boid> &boids, Boid &leader)
{
	// get new acceleration based on 3 rules
	//PVector sep = Seperation(boids);
	
	PVector seek = Seek(leader._position);
	//sep.mult(5);
	//ApplyForce(sep);
	ApplyForce(seek);
}

void Boid::Update()
{
	_velocity.add(_acceleration);
	_velocity.limit(_maxSpeed);
	
	PVector newVelocity = PVector{ _velocity.x / (1240 / 2), _velocity.y / (720 / 2) };
	_position.add(_velocity);
	_acceleration.mult(0); // Reset acceleration
}

void Boid::Borders()
{
	if (_position.x < 0) _position.x = 1280;
	else if (_position.y < 0) _position.y = 720;
	else if (_position.x > 1280) _position.x = 0;
	else if (_position.y > 720) _position.y = 0;
}

void Boid::ApplyForce(PVector force)
{
	_acceleration.add(force);
}

void Boid::Wander()
{
	Update();
	Borders();
}

PVector Boid::Seek(PVector target)
{
	PVector desired = PVector().sub(target, _position);
	desired.normalize();
	desired.mult(_maxSpeed);

	PVector steer = PVector().sub(desired, _velocity);
	steer.limit(_maxForce);
	return steer;	
}

PVector Boid::Seperation(std::vector<Boid> &boids)
{
	float desiredSep = 40.0f;
	PVector steer{ 0,0 };
	int count = 0;
	// check each boid in ssystem, check if its close
	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
	{
		// get distance of neighbor
		float d = PVector().dist(_position, neighborItr->_position);
		if ((d > 0) && (d < desiredSep))
		{
			// Calculate vector point away from neighbor
			PVector diff = PVector().sub(_position, neighborItr->_position);
			diff.normalize();
			diff.div(d); // weight by diff
			steer.add(diff);
			count++; // Keep track of how many
		}
		// Average -- dive by amount
		if (count > 0)
		{
			steer.div((float)count);
		}

		if (steer.mag() > 0)
		{
			steer.normalize();
			steer.mult(_maxSpeed);
			steer.sub(_velocity);
			steer.limit(_maxForce);
		}
		return steer;
	}
}

PVector Boid::Align(std::vector<Boid> &boids)
{
	float neighborDist = 5;
	PVector sum = PVector{ 0, 0 };
	int count = 0;
	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
	{
		float d = PVector().dist(_position, neighborItr->_position);
		if ((d > 0) && (d < neighborDist)) 
		{
			sum.add(neighborItr->_velocity);
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
		sum.mult(_maxSpeed);
		PVector steer = PVector().sub(sum, _velocity);
		steer.limit(_maxForce);
		return steer;
	}
	else {
		return PVector{ 0, 0 };
	}
}

PVector Boid::Cohesion(std::vector<Boid> &boids)
{
	float neighborDist = 5;
	PVector sum = PVector{ 0, 0 };
	int count = 0;
	for (std::vector<Boid>::iterator neighborItr = boids.begin(); neighborItr != boids.end(); ++neighborItr)
	{
		float d = PVector().dist(_position, neighborItr->_position);
		if ((d > 0) && (d < neighborDist))
		{
			sum.add(neighborItr->_position);
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

void Boid::Render()
{

	_vertexData[0].position.x = _position.x / (1280/2) - 1;
	_vertexData[0].position.y = _position.y / (720/2) - 1 + 0.03;

	_vertexData[1].position.x = _position.x / (1280 / 2) - 1 + 0.015;
	_vertexData[1].position.y = _position.y / (720 / 2) - 1 - 0.03;

	_vertexData[2].position.x = _position.x / (1280 / 2) - 1 - 0.015;
	_vertexData[2].position.y = _position.y / (720 / 2) - 1 - 0.03;

	glBindBuffer(GL_ARRAY_BUFFER, _vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_vertexData), _vertexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// bind the buffer object
	glBindBuffer(GL_ARRAY_BUFFER, _vboID);

	// Tell opengl that we want to use the first
	// attribute array. We only need one array right
	// now since we are only using positon.
	glEnableVertexAttribArray(0);

	// This is the position attribute pointer
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,position));
	// This is the color attribute pointer
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

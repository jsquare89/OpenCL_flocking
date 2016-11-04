#include "Boid.h"



Boid::Boid()
{
	Setup(0.0f, 0.0f);
}

void Boid::Setup(float x, float y)
{
	_acceleration = PVector{ 0,0 };
	float angle = (float)(rand() % 360 + 1)*(M_PI / 180);
	_velocity = PVector{ (float)cos(angle),(float)sin(angle) };
	_position = PVector{ x, y };
	_r = 2.0;
	_maxSpeed = 2;
	_maxForce = 0.09;

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

void Boid::Run()
{
	
}

void Boid::Update()
{
	_velocity.add(_acceleration);
}
void Boid::Render()
{
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

#pragma once

struct PVector
{
	void add(PVector vec);
	void limit(float speed);
	void mult(PVector vec);
	float x;
	float y;

};

inline void PVector::add(PVector vec)
{
	x += vec.x;
	y += vec.y;
}

inline void PVector::limit(float speed)
{
	if (x > speed)
		x = speed;
	if (y > speed)
		y = speed;
}

inline void PVector::mult(PVector vec)
{
	x *= vec.x;
	y *= vec.y;
}
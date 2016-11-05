#pragma once
#include <cmath>

struct PVector
{
	void add(PVector vec);
	void limit(float speed);
	void mult(float mul);
	void div(float div);
	void sub(PVector vec);
	float mag();
	PVector sub(PVector first, PVector second);
	float dist(PVector first, PVector second);
	void normalize();
	float x;
	float y;
};

inline void PVector::add(PVector vec)
{
	x += vec.x;
	y += vec.y;
}

inline void PVector::sub(PVector vec)
{
	x -= vec.x;
	y -= vec.y;
}

inline PVector PVector::sub(PVector first, PVector second)
{
	first.x -= second.x;
	first.y -= second.y;
	return first;
}

inline void PVector::limit(float speed)
{
	if (x > speed)
		x = speed;
	if (y > speed)
		y = speed;
}

inline void PVector::mult(float mul)
{
	x *= mul;
	y *= mul;
}

inline float PVector::mag()
{
	return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
}

inline void PVector::normalize()
{
	float mag = this->mag();
	x = x/mag;
	y = y/mag;
}

inline void PVector::div(float div)
{
	x = x / div;
	y = y / div;
}

inline float PVector::dist(PVector first, PVector second)
{
	return std::sqrt(std::pow((first.x - second.x), 2) + std::pow((first.y - second.y), 2));
}
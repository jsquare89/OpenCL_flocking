#pragma once

#include <GL/glew.h>


struct Position {
	float x;
	float y;
};

struct Color { //4 bytes for r g b a color
	GLubyte r;
	GLubyte g;
	GLubyte b;
	GLubyte a;
};

// The vertex definition
struct Vertex {
	Position position;
	Color color;
};
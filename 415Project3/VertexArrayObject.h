#ifndef __VERTEX_ARRAY_OBJECT_H__
#define __VERTEX_ARRAY_OBJECT_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>

using namespace std;

class VertexArrayObject
{

public:

	VertexArrayObject(float length, float width, float depth, GLuint vertposition_loc, GLuint vertcolor_loc);
	~VertexArrayObject();

private:

	void Create();


	GLuint vertex_array;

	GLuint vertex_buffer;
	GLuint color_buffer;
	GLuint index_buffer;

	GLfloat vertex_buffer_data[8];

	GLuint vertposition_loc, vertcolor_loc;
};

#endif
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

	void Create(GLfloat vertex_buffer_data[24]);

	gmtl::Matrix44f matrix;
	GLuint vertex_array, vertposition_loc, vertcolor_loc;
};

#endif
#ifndef __BALL_H__
#define __BALL_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>

using namespace std;

class Ball
{

public:

	Ball();
	Ball(float radius, GLuint vertposition_loc, GLuint vertcolor_loc);
	~Ball();

private:

	void Create(GLfloat vertex_buffer_data[24]);


	gmtl::Matrix44f matrix;

	GLuint vertex_array,
		vertex_buffer, color_buffer, index_buffer,
		vertposition_loc, vertcolor_loc;
};

#endif
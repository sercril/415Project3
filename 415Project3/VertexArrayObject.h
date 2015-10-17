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

	VertexArrayObject(std::vector<GLfloat>* vertex_buffer, std::vector<GLfloat>* color_buffer, std::vector<GLfloat>* index_buffer);
	~VertexArrayObject();




private:
	GLuint vertex_buffer;
	GLuint color_buffer;
	GLuint index_buffer;
};

#endif
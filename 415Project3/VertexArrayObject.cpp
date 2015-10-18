#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>



#include "VertexArrayObject.h"

using namespace std;


static const GLfloat color_buffer_data[] =
{
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f
};

static const GLushort index_buffer_data[] =
{
	0, 4, 1, 5, 3, 7, 2, 6, 0, 4,
	0xFFFF,
	1, 0, 3, 2,
	0xFFFF,
	5, 4, 7, 6,
};

VertexArrayObject::VertexArrayObject(float length, float width, float depth, GLuint vertposition_loc, GLuint vertcolor_loc)
{

	float x1 = 0, x2, y1, y2, z1, z2;

	x2 = length;

	y1 = width / 2;
	y2 = y1 - width;

	z1 = depth / 2;
	z2 = z1 - depth;

	this->vertposition_loc = vertposition_loc;
	this->vertcolor_loc = vertcolor_loc;

	GLfloat vbuffer_data[] =
	{
		x1, y1, z1, //0
		x1, y2, z1, //1
		x1, y1, z2, //2
		x1, y2, z2, //3
		x2, y1, z1, //4
		x2, y2, z1, //5
		x2, y1, z2, //6
		x2, y2, z2 //7
	};

	
	this->Create(vbuffer_data);
	
}

void VertexArrayObject::Create(GLfloat vertex_buffer_data[24])
{
	/*** VERTEX ARRAY OBJECT SETUP***/
	// Create/Generate the Vertex Array Object
	glGenVertexArrays(1, &this->vertex_array);
	// Bind the Vertex Array Object
	glBindVertexArray(this->vertex_array);

	// Create/Generate the Vertex Buffer Object for the vertices.
	glGenBuffers(1, &this->vertex_buffer);
	// Bind the Vertex Buffer Object.
	glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);
	// Specify data location and organization
	glVertexAttribPointer(vertposition_loc, // This number must match the layout in the shader
		3, // Size
		GL_FLOAT, // Type
		GL_FALSE, // Is normalized
		0, ((void*)0));
	// Enable the use of this array
	glEnableVertexAttribArray(vertposition_loc);

	// Similarly, set up the color buffer.
	glGenBuffers(1, &this->color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->color_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(vertcolor_loc, 3, GL_FLOAT, GL_FALSE, 0, ((void*)0));
	glEnableVertexAttribArray(vertcolor_loc);

	// Set up the element (index) array buffer and copy in data
	glGenBuffers(1, &this->index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->index_buffer);
	// Transfer data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(index_buffer_data),
		index_buffer_data, GL_STATIC_DRAW);
}
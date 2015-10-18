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
	this->vertposition_loc = vertposition_loc;
	this->vertcolor_loc = vertcolor_loc;

	this->vertex_buffer = 
	{

	};

	
}

void VertexArrayObject::Create()
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertex_buffer_data), this->vertex_buffer_data, GL_STATIC_DRAW);
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
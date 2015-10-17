#define USE_PRIMITIVE_RESTART
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>

#include "LoadShaders.h"
#include "VertexArrayObject.h"

#pragma comment (lib, "glew32.lib")


using namespace std;


#pragma region "Global Variables"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024
#define NUM_OBJECTS 18

static const GLfloat color_buffer_data[] = 
{
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f
}


#pragma endregion

# pragma region "Mouse Input"

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		mouseX = x;
		mouseY = y;
	}
}

void mouseMotion(int x, int y)
{

	mouseDeltaX = x - mouseX;
	mouseDeltaY = y - mouseY;


	elevation += degreesToRadians(arcToDegrees(mouseDeltaY)) / 1024;
	azimuth += degreesToRadians(arcToDegrees(mouseDeltaX)) / 1024;

	cameraRotate();

	mouseX = x;
	mouseY = y;

}

# pragma endregion

#pragma region "Keyboard Input"

void keyboard(unsigned char key, int x, int y)
{

}

#pragma endregion


void vaoSetup(int vertArrayIndex, ObjectType type)
{
	/*** VERTEX ARRAY OBJECT SETUP***/
	// Create/Generate the Vertex Array Object
	glGenVertexArrays(1, &VertexArrayID[vertArrayIndex]);
	// Bind the Vertex Array Object
	glBindVertexArray(VertexArrayID[vertArrayIndex]);

	// Transfer data in to graphics system
	switch (type)
	{
	case TIP:
		// Create/Generate the Vertex Buffer Object for the vertices.
		glGenBuffers(1, &tipVertexbuffer);
		// Bind the Vertex Buffer Object.
		glBindBuffer(GL_ARRAY_BUFFER, tipVertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tip_vertex_buffer_data), tip_vertex_buffer_data, GL_STATIC_DRAW);
		// Specify data location and organization
		glVertexAttribPointer(vertposition_loc, // This number must match the layout in the shader
			3, // Size
			GL_FLOAT, // Type
			GL_FALSE, // Is normalized
			0, ((void*)0));
		// Enable the use of this array
		glEnableVertexAttribArray(vertposition_loc);

		// Similarly, set up the color buffer.
		glGenBuffers(1, &tipColorbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, tipColorbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tip_color_buffer_data), tip_color_buffer_data, GL_STATIC_DRAW);
		glVertexAttribPointer(vertcolor_loc, 3, GL_FLOAT, GL_FALSE, 0, ((void*)0));
		glEnableVertexAttribArray(vertcolor_loc);

		// Set up the element (index) array buffer and copy in data
		glGenBuffers(1, &tipIndexbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tipIndexbuffer);
		// Transfer data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(tip_indices_buffer_data),
			tip_indices_buffer_data, GL_STATIC_DRAW);
		break;
	case PHALANGE:
		// Create/Generate the Vertex Buffer Object for the vertices.
		glGenBuffers(1, &phalangeVertexbuffer);
		// Bind the Vertex Buffer Object.
		glBindBuffer(GL_ARRAY_BUFFER, phalangeVertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(phalange_vertex_buffer_data), phalange_vertex_buffer_data, GL_STATIC_DRAW);
		// Specify data location and organization
		glVertexAttribPointer(vertposition_loc, // This number must match the layout in the shader
			3, // Size
			GL_FLOAT, // Type
			GL_FALSE, // Is normalized
			0, ((void*)0));
		// Enable the use of this array
		glEnableVertexAttribArray(vertposition_loc);

		// Similarly, set up the color buffer.
		glGenBuffers(1, &phalangeColorbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, phalangeColorbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(phalange_color_buffer_data), phalange_color_buffer_data, GL_STATIC_DRAW);
		glVertexAttribPointer(vertcolor_loc, 3, GL_FLOAT, GL_FALSE, 0, ((void*)0));
		glEnableVertexAttribArray(vertcolor_loc);

		// Set up the element (index) array buffer and copy in data
		glGenBuffers(1, &phalangeIndexbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, phalangeIndexbuffer);
		// Transfer data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(phalange_indices_buffer_data),
			phalange_indices_buffer_data, GL_STATIC_DRAW);
		break;
	case PALM:
		// Create/Generate the Vertex Buffer Object for the vertices.
		glGenBuffers(1, &palmVertexbuffer);
		// Bind the Vertex Buffer Object.
		glBindBuffer(GL_ARRAY_BUFFER, palmVertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(palm_vertex_buffer_data), palm_vertex_buffer_data, GL_STATIC_DRAW);
		// Specify data location and organization
		glVertexAttribPointer(vertposition_loc, // This number must match the layout in the shader
			3, // Size
			GL_FLOAT, // Type
			GL_FALSE, // Is normalized
			0, ((void*)0));
		// Enable the use of this array
		glEnableVertexAttribArray(vertposition_loc);

		// Similarly, set up the color buffer.
		glGenBuffers(1, &palmColorbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, palmColorbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(palm_color_buffer_data), palm_color_buffer_data, GL_STATIC_DRAW);
		glVertexAttribPointer(vertcolor_loc, 3, GL_FLOAT, GL_FALSE, 0, ((void*)0));
		glEnableVertexAttribArray(vertcolor_loc);

		// Set up the element (index) array buffer and copy in data
		glGenBuffers(1, &palmIndexbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, palmIndexbuffer);
		// Transfer data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(palm_indices_buffer_data),
			palm_indices_buffer_data, GL_STATIC_DRAW);
		break;

	case AXIS:
		// Create/Generate the Vertex Buffer Object for the vertices.
		glGenBuffers(1, &axisVertexbuffer);
		// Bind the Vertex Buffer Object.
		glBindBuffer(GL_ARRAY_BUFFER, axisVertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axis_vertex_buffer_data), axis_vertex_buffer_data, GL_STATIC_DRAW);
		// Specify data location and organization
		glVertexAttribPointer(vertposition_loc, // This number must match the layout in the shader
			3, // Size
			GL_FLOAT, // Type
			GL_FALSE, // Is normalized
			0, ((void*)0));
		// Enable the use of this array
		glEnableVertexAttribArray(vertposition_loc);

		// Similarly, set up the color buffer.
		glGenBuffers(1, &axisColorbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, axisColorbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axis_color_buffer_data), axis_color_buffer_data, GL_STATIC_DRAW);
		glVertexAttribPointer(vertcolor_loc, 3, GL_FLOAT, GL_FALSE, 0, ((void*)0));
		glEnableVertexAttribArray(vertcolor_loc);

		// Set up the element (index) array buffer and copy in data
		glGenBuffers(1, &axisIndexbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, axisIndexbuffer);
		// Transfer data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(axis_indices_buffer_data),
			axis_indices_buffer_data, GL_STATIC_DRAW);
		break;
	}
}

void display()
{

	gmtl::Matrix44f thisTransform;

	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < NUM_MATRICES; ++i)
	{
		thisTransform = view * objects[i];
		glBindVertexArray(VertexArrayID[i]);
		// Send a different transformation matrix to the shader
		glUniformMatrix4fv(Matrix_loc, 1, GL_FALSE, &thisTransform[0][0]);

		// Draw the transformed cuboid
		glEnable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(0xFFFF);
		glDrawElements(GL_TRIANGLE_STRIP, indexCount[PALM], GL_UNSIGNED_SHORT, NULL);
	}

	//Ask GL to execute the commands from the buffer
	glFlush();	// *** if you are using GLUT_DOUBLE, use glutSwapBuffers() instead 

	//Check for any errors and print the error string.
	//NOTE: The string returned by gluErrorString must not be altered.
	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		cout << "OpengGL Error: " << errString << endl;
	}
}


void init()
{

	ObjectType type;

	elevation = azimuth = 0;

	// Enable depth test (visible surface determination)
	glEnable(GL_DEPTH_TEST);

	// OpenGL background color
	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

	// Load/compile/link shaders and set to use for rendering
	ShaderInfo shaders[] = { { GL_VERTEX_SHADER, "Cube_Vertex_Shader.vert" },
	{ GL_FRAGMENT_SHADER, "Cube_Fragment_Shader.frag" },
	{ GL_NONE, NULL } };

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);

	//Get the shader parameter locations for passing data to shaders
	vertposition_loc = glGetAttribLocation(program, "vertexPosition");
	vertcolor_loc = glGetAttribLocation(program, "vertexColor");
	Matrix_loc = glGetUniformLocation(program, "Matrix");


	view.set(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);


	for (int i = 0; i < NUM_MATRICES; ++i)
	{
		if (i < 5)
		{
			type = TIP;
		}
		else if (i >= 5 && i < 10)
		{
			type = PHALANGE;
		}
		else if (i == 10)
		{
			type = PALM;
		}
		else
		{
			type = AXIS;
		}

		switch (type)
		{

		case TIP:
		case PALM:
		case AXIS:
			objects[i].set(
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1);
			break;
		case PHALANGE:
			objects[i].set(
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1);
			activeMatrixIndex = i;
			worldTranslate(0.5f, 0.0f, 0.0f);
			break;
		}

		vaoSetup(i, type);
	}

	activeMatrixIndex = 10;
}

int main(int argc, char** argv)
{
	// For more details about the glut calls, 
	// refer to the OpenGL/freeglut Introduction handout.

	//Initialize the freeglut library
	glutInit(&argc, argv);

	//Specify the display mode
	glutInitDisplayMode(GLUT_RGBA);

	//Set the window size/dimensions
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Specify OpenGL version and core profile.
	// We use 3.3 in thie class, not supported by very old cards
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("415/515 CUBOID DEMO");

	glewExperimental = GL_TRUE;

	if (glewInit())
		exit(EXIT_FAILURE);

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);

	//Transfer the control to glut processing loop.
	glutMainLoop();

	return 0;
}
#define USE_PRIMITIVE_RESTART
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <list>
#include <sstream>
#include <fstream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>

#include "LoadShaders.h"
#include "VertexArrayObject.h"

#pragma comment (lib, "glew32.lib")
#pragma warning (disable : 4996) // Windows ; consider instead replacing fopen with fopen_s

using namespace std;

#pragma region Structs and Enums

enum ObjectType
{
	FLOOR = 0,
	BALL,
	PALM,
	METACARPAL,
	PROXIMAL,
	MIDDLE,
	DISTAL
};

struct SceneNode
{	
	VertexArrayObject vao;
	ObjectType type;
	SceneNode* parent;
	std::vector<SceneNode *> children;

	SceneNode()
	{

	}
};


struct Keyframe
{
	unsigned long time; // Timestamp, milliseconds since first record. Assume nondecreasing order.
	float palm_p[3];    // palm position w.r.t. world (x, y, z)
	float palm_q[4];    // palm orientation w.r.t. world, quaternion (a, b, c, s) a.k.a. (x, y, z, w)
	float joint[16];    // finger joint angles (in radians). See list above.
	float ball_p[3];    // ball position
	float ball_q[4];    // ball orientation
};


#pragma endregion

#pragma region "Global Variables"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024
#define NUM_OBJECTS 18
#define BOX_INDECIES 20


// Parameter location for passing a matrix to vertex shader
GLuint Matrix_loc;
// Parameter locations for passing data to shaders
GLuint vertposition_loc, vertcolor_loc;

GLenum errCode;
const GLubyte *errString;

int mouseX, mouseY,
mouseDeltaX, mouseDeltaY;

float azimuth, elevation;

gmtl::Matrix44f view;

std::vector<SceneNode> sceneGraph;

float hand[16][3], ballRadius, floorY, attachments[4], thumbLoc[3];


#pragma endregion


#pragma region Helper Functions

float arcToDegrees(float arcLength)
{
	return ((arcLength * 360.0f) / (2.0f * M_PI));
}

void cameraRotate()
{
	gmtl::Matrix44f elevationRotation, azimuthRotation;
	
	elevationRotation = gmtl::makeRot<gmtl::Matrix44f>(gmtl::EulerAngleXYZf((gmtl::Math::deg2Rad(elevation) / SCREEN_HEIGHT)*-1, 0.0f, 0.0f));
	azimuthRotation = gmtl::makeRot<gmtl::Matrix44f>(gmtl::EulerAngleXYZf(0.0f, (gmtl::Math::deg2Rad(elevation) / SCREEN_HEIGHT)*-1, 0.0f));

	elevationRotation.setState(gmtl::Matrix44f::ORTHOGONAL);

	azimuthRotation.setState(gmtl::Matrix44f::ORTHOGONAL);

	view = azimuthRotation * elevationRotation;

	view.setState(gmtl::Matrix44f::ORTHOGONAL);

	gmtl::transpose(view);

	glutPostRedisplay();
}

void readGeometry()
{
	ifstream fp;
	int i = 0;

	fp.open("geometryfileexample.txt", ios_base::in);

	if (fp)
	{
		for (std::string line; std::getline(fp, line); ++i)
		{
			std::istringstream in(line);

			if (i < 16)
			{
				in >> hand[i][0] >> hand[i][1] >> hand[i][2];
			}
			else if (i == 16)
			{
				in >> thumbLoc[0] >> thumbLoc[1] >> thumbLoc[2];
			}
			else if (i == 17)
			{
				in >> attachments[0] >> attachments[1] >> attachments[2] >> attachments[3];
			}
			else if (i == 18)
			{
				in >> ballRadius;
			}
			else if (i == 19)
			{
				in >> floorY;
			}
			
		}
		

		fp.close();
	}
}

void buildFinger(int finger, SceneNode* newNode)
{
	SceneNode* metacarpal;
	SceneNode* proximal;
	SceneNode* middle;
	SceneNode* distal;

	metacarpal = proximal = middle = distal = new SceneNode();

	if (finger == 0)
	{
		metacarpal->parent = &sceneGraph[0];
		metacarpal->type = METACARPAL;
		metacarpal->vao = VertexArrayObject(hand[finger + 1][0], hand[finger + 1][1], hand[finger + 1][2], vertposition_loc, vertcolor_loc);

		proximal->type = PROXIMAL;
		proximal->parent = metacarpal;
		proximal->vao = VertexArrayObject(hand[finger + 2][0], hand[finger + 2][1], hand[finger + 2][2], vertposition_loc, vertcolor_loc);

		distal->type = DISTAL;
		distal->parent = proximal;
		distal->vao = VertexArrayObject(hand[finger + 3][0], hand[finger + 3][1], hand[finger + 3][2], vertposition_loc, vertcolor_loc);
		distal->children.clear();

		proximal->children[0] = distal;
		metacarpal->children[0] = proximal;
		
		newNode = metacarpal;
	}
	else
	{
		proximal->type = PROXIMAL;
		proximal->parent = &sceneGraph[0];
		proximal->vao = VertexArrayObject(hand[finger + 1][0], hand[finger + 1][1], hand[finger + 1][2], vertposition_loc, vertcolor_loc);

		middle->type = MIDDLE;
		middle->parent = metacarpal;
		middle->vao = VertexArrayObject(hand[finger + 2][0], hand[finger + 2][1], hand[finger + 2][2], vertposition_loc, vertcolor_loc);

		distal->type = DISTAL;
		distal->parent = proximal;
		distal->vao = VertexArrayObject(hand[finger + 3][0], hand[finger + 3][1], hand[finger + 3][2], vertposition_loc, vertcolor_loc);
		distal->children.clear();

		middle->children[0] = distal;
		proximal->children[0] = proximal;

		newNode = proximal;
	}

	return;
}

void buildGraph()
{
	
	SceneNode node;

	//Hand
	node.type = PALM;
	node.parent = NULL;
	node.vao = VertexArrayObject(hand[0][0], hand[0][1], hand[0][2], vertposition_loc, vertcolor_loc);
	
	for (int i = 0; i < 5; ++i)
	{
		buildFinger(i, node.children[0]);		
	}
	
	sceneGraph[0] = node;
	//Ball
	
	//Floor
	
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


	elevation += arcToDegrees(mouseDeltaY);
	azimuth += arcToDegrees(mouseDeltaX);

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

void display()
{

	//gmtl::Matrix44f thisTransform;

	//// Clear the color and depth buffers
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//for (int i = 0; i < NUM_MATRICES; ++i)
	//{
	//	thisTransform = view * objects[i];
	//	glBindVertexArray(VertexArrayID[i]);
	//	// Send a different transformation matrix to the shader
	//	glUniformMatrix4fv(Matrix_loc, 1, GL_FALSE, &thisTransform[0][0]);

	//	// Draw the transformed cuboid
	//	glEnable(GL_PRIMITIVE_RESTART);
	//	glPrimitiveRestartIndex(0xFFFF);
	//	glDrawElements(GL_TRIANGLE_STRIP, BOX_INDECIES, GL_UNSIGNED_SHORT, NULL);
	//}

	////Ask GL to execute the commands from the buffer
	//glFlush();	// *** if you are using GLUT_DOUBLE, use glutSwapBuffers() instead 

	////Check for any errors and print the error string.
	////NOTE: The string returned by gluErrorString must not be altered.
	//if ((errCode = glGetError()) != GL_NO_ERROR) {
	//	errString = gluErrorString(errCode);
	//	cout << "OpengGL Error: " << errString << endl;
	//}
}


void init()
{



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

	gmtl::identity(view);


	
}

int main(int argc, char** argv)
{

	readGeometry();
	system("PAUSE");
	return 0;


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
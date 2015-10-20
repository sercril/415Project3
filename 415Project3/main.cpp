#define USE_PRIMITIVE_RESTART
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <fstream>
#include <stack>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>

#include "LoadShaders.h"
#include "SceneObject.h"

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
	SceneObject object;
	ObjectType type;
	SceneNode* parent;
	std::vector<SceneNode *> children;
	int id;

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
//#define WORLD_OBJECTS 3
#define NUM_OBJECTS 18
#define INDECIES 3000

// Parameter location for passing a matrix to vertex shader
GLuint Matrix_loc;
// Parameter locations for passing data to shaders
GLuint vertposition_loc, vertcolor_loc;

GLenum errCode;
const GLubyte *errString;

int mouseX, mouseY,
mouseDeltaX, mouseDeltaY;

float azimuth, elevation;

gmtl::Matrix44f view, modelView, ballTransform, floorTransform, palmTransform;


std::vector<SceneNode*> sceneGraph;

float hand[16][3], ballRadius, floorY, attachments[4], thumbLoc[3];

std::vector<GLfloat> ball_vertex_data;
std::vector<GLushort> ball_index_data;

std::vector<gmtl::Matrix44f> fingerTransforms[15];

#pragma endregion


#pragma region Helper Functions

float arcToDegrees(float arcLength)
{
	return ((arcLength * 360.0f) / (2.0f * M_PI));
}

void cameraRotate()
{
	gmtl::Matrix44f elevationRotation, azimuthRotation, viewScale;
	
	elevationRotation = gmtl::makeRot<gmtl::Matrix44f>(gmtl::EulerAngleXYZf((gmtl::Math::deg2Rad(elevation) / SCREEN_HEIGHT)*-1, 0.0f, 0.0f));
	azimuthRotation = gmtl::makeRot<gmtl::Matrix44f>(gmtl::EulerAngleXYZf(0.0f, (gmtl::Math::deg2Rad(azimuth) / SCREEN_WIDTH)*-1, 0.0f));

	elevationRotation.setState(gmtl::Matrix44f::ORTHOGONAL);

	azimuthRotation.setState(gmtl::Matrix44f::ORTHOGONAL);

	/*viewScale = gmtl::makeScale<gmtl::Matrix44f>(gmtl::Vec3f(-2.0f,-2.0f, -2.0f));

	viewScale.setState(gmtl::Matrix44f::AFFINE);*/

	view = azimuthRotation * elevationRotation;// *viewScale;

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

void importBallData()
{
	ifstream fp;
	int i = 0, j = 0, numVerticies, numIndicies, numPolygons;

	fp.open("SphereMesh.txt", ios_base::in);

	if (fp)
	{

		for (std::string line; std::getline(fp, line); ++i)
		{
			std::istringstream in(line);

			if (i == 0)
			{
				in >> numVerticies;
				ball_vertex_data.resize(numVerticies*3);
			}
			else if (i > 0 && i <= numVerticies)
			{
				in >> ball_vertex_data[j] >> ball_vertex_data[j+1] >> ball_vertex_data[j+2];
				j += 3;
			}
			else if (i == (numVerticies + 1))
			{
				in >> numPolygons;
				ball_index_data.resize(numPolygons*3);
				j = 0;
			}
			else if (i > (numVerticies + 1))
			{
				in >> numIndicies >> ball_index_data[j] >> ball_index_data[j + 1] >> ball_index_data[j + 2];
				j+=3;
			}
		}


		fp.close();
	}
}

SceneNode * buildFinger(int finger)
{
	SceneNode* metacarpal = new SceneNode();
	SceneNode* proximal = new SceneNode();
	SceneNode* middle = new SceneNode();
	SceneNode* distal = new SceneNode();
	SceneNode* returnNode = new SceneNode();
	int base;
	gmtl::Matrix44f initialTranslation;

	if (finger == 0)
	{
		metacarpal->parent = sceneGraph[0];
		metacarpal->type = METACARPAL;
		metacarpal->object = SceneObject(hand[finger + 1][0], hand[finger + 1][1], hand[finger + 1][2], vertposition_loc, vertcolor_loc);
		metacarpal->id = finger;
		initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(thumbLoc[0], thumbLoc[1], thumbLoc[2]));
		initialTranslation.setState(gmtl::Matrix44f::TRANS);
		metacarpal->object.matrix = metacarpal->parent->object.matrix * initialTranslation;

		proximal->type = PROXIMAL;
		proximal->parent = metacarpal;
		proximal->object = SceneObject(hand[finger + 2][0], hand[finger + 2][1], hand[finger + 2][2], vertposition_loc, vertcolor_loc);
		proximal->id = finger + 1;
		initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(proximal->parent->object.length, 0.0f, 0.0f));
		initialTranslation.setState(gmtl::Matrix44f::TRANS);
		proximal->object.matrix = proximal->parent->object.matrix * initialTranslation;

		distal->type = DISTAL;
		distal->parent = proximal;
		distal->object = SceneObject(hand[finger + 3][0], hand[finger + 3][1], hand[finger + 3][2], vertposition_loc, vertcolor_loc);
		distal->children.clear();
		distal->id = finger + 2;
		initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(distal->parent->object.length, 0.0f, 0.0f));
		initialTranslation.setState(gmtl::Matrix44f::TRANS);
		distal->object.matrix = distal->parent->object.matrix * initialTranslation;

		proximal->children.push_back(distal);
		metacarpal->children.push_back(proximal);
		
		returnNode = metacarpal;

	}
	else
	{
		base = finger * 3;

		proximal->type = PROXIMAL;
		proximal->parent = sceneGraph[0];
		proximal->object = SceneObject(hand[base + 1][0], hand[base + 1][1], hand[base + 1][2], vertposition_loc, vertcolor_loc);
		proximal->id = base;
		initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0.0f, attachments[finger], 0.0f));
		initialTranslation.setState(gmtl::Matrix44f::TRANS);
		proximal->object.matrix = proximal->parent->object.matrix * initialTranslation;

		middle->type = MIDDLE;
		middle->parent = metacarpal;
		middle->object = SceneObject(hand[base + 2][0], hand[base + 2][1], hand[base + 2][2], vertposition_loc, vertcolor_loc);
		middle->id = base + 1;
		initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(middle->parent->object.length, 0.0f, 0.0f));
		initialTranslation.setState(gmtl::Matrix44f::TRANS);
		middle->object.matrix = middle->parent->object.matrix * initialTranslation;

		distal->type = DISTAL;
		distal->parent = proximal;
		distal->object = SceneObject(hand[base + 3][0], hand[base + 3][1], hand[base + 3][2], vertposition_loc, vertcolor_loc);
		distal->children.clear();
		distal->id = base + 2;
		initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(distal->parent->object.length, 0.0f, 0.0f));
		initialTranslation.setState(gmtl::Matrix44f::TRANS);
		distal->object.matrix = distal->parent->object.matrix * initialTranslation;

		middle->children.push_back(distal);
		proximal->children.push_back(middle);

		returnNode = proximal;

	}

	return returnNode;
}

void buildGraph()
{
	
	SceneNode* palm = new SceneNode();
	SceneNode* ball = new SceneNode();
	SceneNode* floor = new SceneNode();
	gmtl::Matrix44f initialTranslation;

	readGeometry();

	//Hand
	palm->type = PALM;
	palm->parent = NULL;
	palm->object = SceneObject(hand[0][0], hand[0][1], hand[0][2], vertposition_loc, vertcolor_loc);
	palm->children = std::vector<SceneNode*>(5);
		
	sceneGraph.push_back(palm);
	
	for (int i = 0; i < 5; ++i)
	{		
		palm->children[i] = buildFinger(i);
	}
	
	

	//Ball
	importBallData();
	ball->type = BALL;
	ball->parent = NULL;
	ball->object = SceneObject(ballRadius, ball_vertex_data, ball_index_data, vertposition_loc, vertcolor_loc);
	ball->children.clear();
	sceneGraph.push_back(ball);

	//Floor
	floor->type = FLOOR;
	floor->parent = NULL;
	floor->object = SceneObject(ballRadius * 3, 3.0f, ballRadius, vertposition_loc, vertcolor_loc);
	floor->children.clear();
	initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0.0f, floorY, 0.0f));
	initialTranslation.setState(gmtl::Matrix44f::TRANS);
	floor->object.matrix = floor->object.matrix * initialTranslation;

	sceneGraph.push_back(floor);
}

void renderGraph(std::vector<SceneNode*> graph)
{
	gmtl::Matrix44f thisTransform, renderTransform, inverseTransform;

	if(!graph.empty())
	{
		for (int i = 0; i < 1; ++i)
		{
			switch (graph[i]->type)
			{
				case PALM:

					graph[i]->object.matrix = palmTransform * graph[i]->object.matrix;
					modelView = modelView * graph[i]->object.matrix;
					break;
				case METACARPAL:
				case PROXIMAL:
				case MIDDLE:
				case DISTAL:
					for (int j = (fingerTransforms[graph[i]->id].size() - 1); j >=0; --j)
					{
						thisTransform = thisTransform * fingerTransforms[graph[i]->id][j];
					}
					graph[i]->object.matrix = graph[i]->object.matrix * thisTransform;
					modelView = modelView * graph[i]->object.matrix;
					break;
				case BALL:
					graph[i]->object.matrix = ballTransform * graph[i]->object.matrix;
					modelView = modelView * graph[i]->object.matrix;
					break;
				case FLOOR:
					graph[i]->object.matrix = floorTransform * graph[i]->object.matrix;
					modelView = modelView * graph[i]->object.matrix;
					break;
			}

			//Render
			renderTransform = modelView;
			glBindVertexArray(graph[i]->object.vertex_array);
			// Send a different transformation matrix to the shader
			glUniformMatrix4fv(Matrix_loc, 1, GL_FALSE, &renderTransform[0][0]);

			// Draw the transformed cuboid
			glEnable(GL_PRIMITIVE_RESTART);
			glPrimitiveRestartIndex(0xFFFF);
			glDrawElements(GL_TRIANGLE_STRIP, INDECIES, GL_UNSIGNED_SHORT, NULL);


			
			/*if (!graph[i]->children.empty())
			{
				renderGraph(graph[i]->children);
				gmtl::invert(inverseTransform, graph[i]->object.matrix * thisTransform);
				modelView = modelView * inverseTransform;
			}*/
			
		}
	}
	
	return;
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


	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	renderGraph(sceneGraph);

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
	gmtl::identity(modelView);
	gmtl::identity(ballTransform);
	gmtl::identity(floorTransform);
	gmtl::identity(palmTransform);

	buildGraph();
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
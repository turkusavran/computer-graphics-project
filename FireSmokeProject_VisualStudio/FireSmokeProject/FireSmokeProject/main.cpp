/*
*	INCLUDES
*/
#include <stdio.h>
#include <math.h>
#include <iostream>
#include "Angel.h"

using namespace std;

//----------------------------------------------------------------------------

/*
*	GLOBAL
*/

// Adjusting the window width and height
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800
#define	MAX_PARTICLES 10000		// Number Of Particles To Create

// TypeDef
typedef vec3 point3;
typedef vec4 point4;
typedef vec4 color4;

// Initialize shader lighting parameters
point4 light_position(-1.0, 0.0, 0.0, 1.0);
color4 light_ambient(0.2, 0.2, 0.2, 1.0);		// L_a
color4 light_diffuse(1.0, 1.0, 1.0, 1.0);		// L_d
color4 light_specular(1.0, 1.0, 1.0, 1.0);		// L_s

// Texture objects and storage for texture image
GLuint textures[2];		// Two textures will be stored: fire & smoke
GLubyte* image;
vec3 texture_coordinates;
int fireParticleWidth, fireParticleHeight;
int smokeParticleWidth, smokeParticleHeight;

point4 points[6];
vec3 normals[2];

int textureType = 0;	// 0: For Fire, 1: For Smoke



GLuint g_mainWnd;

int numKeys = 0;
bool keys[4];

float	xspeed;				// Base X Speed (To Allow Keyboard Direction Of Tail)
float	yspeed;				// Base Y Speed (To Allow Keyboard Direction Of Tail)
float	zoom = -40.0f;		// Used To Zoom Out

GLuint	loop;				// Misc Loop Variable

// Model-view and projection matrices and ShadingType uniform location
GLuint  ModelView, Projection, TextureType;

//----------------------------------------------------------------------------

/*
*	RGBA COLORS
*/

// Total of 12 colors
color4 vertex_colors[12] =
{
	color4(0.0, 0.0, 0.0, 1.0),			// Black
	color4(0.54, 0.27, 0.07, 1.0),		// Brown
	color4(0.29, 0.0, 0.51, 1.0),		// Purple
	color4(1.0, 0.078, 0.57, 1.0),		// Pink
	color4(0.0, 0.0, 1.0, 1.0),			// Blue
	color4(0.0, 1.0, 0.0, 1.0),			// Green
	color4(1.0, 1.0, 0.0, 1.0),			// Yellow
	color4(1.0, 0.55, 0.0, 1.0),		// Orange
	color4(1.0, 0.0, 0.0, 1.0),			// Red
	color4(1.0, 1.0, 1.0, 1.0),			// White
	color4(1.0, 0.0, 1.0, 1.0),			// Magenta
	color4(0.0, 1.0, 1.0, 1.0)			// Cyan
};

//----------------------------------------------------------------------------
/*
*	PARTICLE EFFECTS
*/

float posX = 0.0f;
float posY = -5.0f;
float posZ = 0.0f;

float gravX = 0.0f;
float gravY = 0.0f;
float gravZ = 0.0f;

float camX = 0.0f;
float camY = 2.5f;
float camZ = 50.0f;

typedef struct						// Create A Structure For Particle
{
	bool	active;					// Active (Yes/No)
	float	life;					// Particle Life
	float	fade;					// Fade Speed
	float	r;						// Red Value
	float	g;						// Green Value
	float	b;						// Blue Value
	float	x;						// X Position
	float	y;						// Y Position
	float	z;						// Z Position
	float	xi;						// X Direction
	float	yi;						// Y Direction
	float	zi;						// Z Direction
	float	xg;						// X Gravity
	float	yg;						// Y Gravity
	float	zg;						// Z Gravity
}
particles;							// Particles Structure

particles particle[MAX_PARTICLES];	// Particle Array (Room For Particle Info)

//----------------------------------------------------------------------------

/*
*	INITIALIZE
*/

void init()
{

	// Print out OpenGL version
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

	// TODO: Create quad

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(texture_coordinates), NULL, GL_STATIC_DRAW);

	// Specify an offset to keep track of where we're placing data in our
	// vertex array buffer.  We'll use the same technique when we
	// associate the offsets with vertex attribute pointers.
	GLintptr offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(points), points);
	offset += sizeof(points);

	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals), normals);
	offset += sizeof(normals);

	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(texture_coordinates), texture_coordinates);

	// Initialize texture object
	glGenTextures(2, textures);

	// Fire
	// TODO: Read bitmap file
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fireParticleWidth, fireParticleHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Smoke
	// TODO: Read bitmap file
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, smokeParticleWidth, smokeParticleHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Using InitShader to setup shader files
	GLuint shaderProgram = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(shaderProgram);

	/*
	*	Set up vertex arrays
	*/
	offset = 0;
	// vPosition
	GLuint vPosition = glGetAttribLocation(shaderProgram, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
	offset += sizeof(points);

	// vNormal
	GLuint vNormal = glGetAttribLocation(shaderProgram, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
	offset += sizeof(normals);

	// vCoords
	GLuint vTexCoord = glGetAttribLocation(shaderProgram, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(shaderProgram, "ModelView");
	Projection = glGetUniformLocation(shaderProgram, "Projection");
	TextureType = glGetUniformLocation(shaderProgram, "TextureType");

	// Set the textureType value to 0 (default is phong shading)
	glUniform1i(TextureType, textureType);

	glEnable(GL_DEPTH_TEST);

	// Enable culling
	glEnable(GL_CULL_FACE);

	// White background
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------
/*
*	DISPLAY FUNCTION
*/

void display()
{

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);					// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup

	glDisable(GL_DEPTH_TEST);							// Disable the depth test for 2D							

	glEnable(GL_BLEND);									// Enable Blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);					// Type Of Blending To Perform
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);				// Really Nice Point Smoothing
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glBindTexture(GL_TEXTURE_2D, textures[0]);			// Select Our Texture

	for (loop = 0;loop < MAX_PARTICLES;loop++)				// Initials All The Textures
	{
		particle[loop].active = true;								// Make All The Particles Active
		particle[loop].life = 1.0f;								// Give All The Particles Full Life
		particle[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;	// Random Fade Speed
		particle[loop].r = vertex_colors[9][0];
		particle[loop].g = vertex_colors[9][1];
		particle[loop].b = vertex_colors[9][2];
		particle[loop].xi = float((rand() % 50) - 25.0f) * 10.0f;		// Random Speed On X Axis
		particle[loop].yi = float((rand() % 50) - 25.0f) * 10.0f;		// Random Speed On Y Axis
		particle[loop].zi = float((rand() % 50) - 25.0f) * 10.0f;		// Random Speed On Z Axis
		particle[loop].xg = 0.0f;									// Set Horizontal Pull 
		particle[loop].yg = 0.8f;									// Set Vertical Pull 
		particle[loop].zg = 0.0f;									// Set Pull On Z Axis To Zero
	}
}

//// Set Light
//void SetLight()
//{
//	float direction[] = { 0.0f, 0.0f, 1.0f, 0.0f };
//	float diffintensity[] = { 0.7f, 0.7f, 0.7f, 1.0f };
//	float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
//
//	glLightfv(GL_LIGHT0, GL_POSITION, direction);
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffintensity);
//	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
//
//
//	glEnable(GL_LIGHT0);
//}

//----------------------------------------------------------------------------

/*
*	RESHAPE FUNCTION
*/

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	GLfloat left = -2.0, right = 2.0;
	GLfloat top = 2.0, bottom = -2.0;
	GLfloat zNear = -20.0, zFar = 20.0;

	GLfloat aspect = GLfloat(width) / height;

	if (aspect > 1.0) {
		left *= aspect;
		right *= aspect;
	}
	else {
		top /= aspect;
		bottom /= aspect;
	}

	mat4 projection = Ortho(left, right, bottom, top, zNear, zFar);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

}

//----------------------------------------------------------------------------

//// Set Camera Position
//void SetCamera()
//{
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	// gluPerspective(G308_FOVY, (double)g_nWinWidth / (double)g_nWinHeight, G308_ZNEAR_3D, G308_ZFAR_3D);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	gluLookAt(camX, camY, camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
//
//}

void DrawGLScene()										// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glLoadIdentity();										// Reset The ModelView Matrix
	for (loop = 0;loop < MAX_PARTICLES;loop++)					// Loop Through All The Particles
	{
		if (particle[loop].active)							// If The Particle Is Active
		{

			float x = particle[loop].x;						// Grab Our Particle X Position
			float y = particle[loop].y;						// Grab Our Particle Y Position
			float z = particle[loop].z + zoom;					// Particle Z Pos + Zoom

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(particle[loop].r, particle[loop].g, particle[loop].b, particle[loop].life);

			glBegin(GL_TRIANGLE_STRIP);						// Build Quad From A Triangle Strip
			glTexCoord2d(1, 1); glVertex3f(x + 0.5f, y + 0.5f, z); // Top Right
			glTexCoord2d(0, 1); glVertex3f(x - 0.5f, y + 0.5f, z); // Top Left
			glTexCoord2d(1, 0); glVertex3f(x + 0.5f, y - 0.5f, z); // Bottom Right
			glTexCoord2d(0, 0); glVertex3f(x - 0.5f, y - 0.5f, z); // Bottom Left
			glEnd();										// Done Building Triangle Strip

			particle[loop].x += particle[loop].xi / (750);// Move On The X Axis By X Speed
			particle[loop].y += particle[loop].yi / (1000);// Move On The Y Axis By Y Speed
			particle[loop].z += particle[loop].zi / (750);// Move On The Z Axis By Z Speed

			if ((particle[loop].x > posX) && (particle[loop].y > (0.1 + posY))) {
				particle[loop].xg = -0.3f;
			}
			else if ((particle[loop].x < posX) && (particle[loop].y > (0.1 + posY))) {
				particle[loop].xg = 0.3f;
			}
			else {
				particle[loop].xg = 0.0f;
			}

			particle[loop].xi += (particle[loop].xg + gravX);			// Take Pull On X Axis Into Account
			particle[loop].yi += (particle[loop].yg + gravY);			// Take Pull On Y Axis Into Account
			particle[loop].zi += (particle[loop].zg + gravZ);			// Take Pull On Z Axis Into Account
			particle[loop].life -= particle[loop].fade;		// Reduce Particles Life By 'Fade'

			if (particle[loop].life < 0.0f)					// If Particle Is Burned Out
			{
				particle[loop].life = 1.0f;					// Give It New Life
				particle[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;	// Random Fade Value
				particle[loop].x = posX;						// Center On X Axis
				particle[loop].y = posY;						// Center On Y Axis
				particle[loop].z = posZ;						// Center On Z Axis
				particle[loop].xi = xspeed + float((rand() % 60) - 30.0f);	// X Axis Speed And Direction
				particle[loop].yi = yspeed + float((rand() % 60) - 30.0f);	// Y Axis Speed And Direction
				particle[loop].zi = float((rand() % 60) - 30.0f);	// Z Axis Speed And Direction
				particle[loop].r = vertex_colors[9][0];		// White color
				particle[loop].g = vertex_colors[9][1];
				particle[loop].b = vertex_colors[9][2];
			}
			else if (particle[loop].life < 0.4f)
			{
				particle[loop].r = vertex_colors[8][0];		// Red color
				particle[loop].g = vertex_colors[8][1];
				particle[loop].b = vertex_colors[8][2];
			}
			else if (particle[loop].life < 0.6f)
			{
				particle[loop].r = vertex_colors[7][0];		// Orange color
				particle[loop].g = vertex_colors[7][1];
				particle[loop].b = vertex_colors[7][2];
			}
			else if (particle[loop].life < 0.75f)
			{
				particle[loop].r = vertex_colors[6][0];		// Yellow color
				particle[loop].g = vertex_colors[6][1];
				particle[loop].b = vertex_colors[6][2];
			}
			else if (particle[loop].life < 0.85f)
			{
				particle[loop].r = vertex_colors[4][0];		// Blue color
				particle[loop].g = vertex_colors[4][1];
				particle[loop].b = vertex_colors[4][2];
			}
		}
	}
	glutPostRedisplay();
	glutSwapBuffers();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	g_mainWnd = glutCreateWindow("Fire & Smoke Particles");

	glutDisplayFunc(DrawGLScene);
	// glutReshapeFunc(reshape);

	init();

	// SetLight();
	// SetCamera();

	glutMainLoop();

	return 0;
}

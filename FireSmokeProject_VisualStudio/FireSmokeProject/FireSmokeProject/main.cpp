//
//  fireSmoke.cpp
//  FireSmokeProject
//
//  Created by Türkü Savran on 13.06.2021.
//
#define GL_SILENCE_DEPRECATION
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "define.h"
#include "Angel.h"

// TypeDef
typedef vec3 point3;
typedef vec4 point4;
typedef vec4 color4;

using namespace std;

#define MAX_PARTICLES 10000        // Number Of Particles To Create

GLuint g_mainWnd;
GLuint g_nWinWidth = G308_WIN_WIDTH;
GLuint g_nWinHeight = G308_WIN_HEIGHT;

int windowWidth = 800, windowHeight = 800;

int numKeys = 0;
bool keys[4];

float xspeed;                        // Base X Speed (To Allow Keyboard Direction Of Tail)
float yspeed;                        // Base Y Speed (To Allow Keyboard Direction Of Tail)
float zoom = -40.0f;                // Used To Zoom Out

GLuint loop;                        // Misc Loop Variable

// Stores the fire particle bmp image at 0
// Stores the smoke particle bmp image at 1
GLuint textures[2];
int textureWidth, textureHeight;
GLubyte* image;

GLuint particleColor;

static GLfloat white[3] = { 1.0f,1.0f,1.0f };
static GLfloat blue[3] = { 0.0f,0.0f,1.0f };
static GLfloat yellow[3] = { 1.0f,1.0f,0.0f };
static GLfloat orange[3] = { 1.0f,0.5f,0.0f };
static GLfloat red[3] = { 1.0f,0.1f,0.0f };

float posX = 0.0f;
float posY = -5.0f;
float posZ = 0.0f;

float gravX = 0.0f;
float gravY = 0.0f;
float gravZ = 0.0f;

float camX = 0.0f;
float camY = 2.5f;
float camZ = 50.0f;

typedef struct                        // Create A Structure For Particle
{
    bool    active;                    // Active (Yes/No)
    float    life;                    // Particle Life
    float    fade;                    // Fade Speed
    float    r;                        // Red Value
    float    g;                        // Green Value
    float    b;                        // Blue Value
    float    x;                        // X Position
    float    y;                        // Y Position
    float    z;                        // Z Position
    float    xi;                        // X Direction
    float    yi;                        // Y Direction
    float    zi;                        // Z Direction
    float    xg;                        // X Gravity
    float    yg;                        // Y Gravity
    float    zg;                        // Z Gravity
}
particles;                            // Particles Structure

particles particle[MAX_PARTICLES];    // Particle Array (Room For Particle Info)

void LoadTexture(const char* filename)
{
    unsigned char* data;

    FILE* file;
    file = fopen(filename, "rb");

    textureWidth = 32;
    textureHeight = 32;
    data = (unsigned char*)malloc(textureWidth * textureHeight * 3);
    //int size = fseek(file,);
    fread(data, textureWidth * textureHeight * 3, 1, file);
    fclose(file);

    for (int i = 0; i < textureWidth * textureWidth; ++i)
    {
        int index = i * 3;
        unsigned char B, R;
        B = data[index];
        R = data[index + 2];

        data[index] = R;
        data[index + 2] = B;
    }
    image = data;
}

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

void InitGL()
{
    /*GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);*/

    // Loading the fire particle bmp file
    LoadTexture("particle.bmp");
    glGenTextures(1, &textures[0]);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    // Loading the smoke particle bmp file
    LoadTexture("smoke.bmp");
    glGenTextures(1, &textures[1]);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);


    // GEREK VAR MI? glShadeModel(GL_SMOOTH);                            // Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);                    // Black Background
    glClearDepth(1.0f);                                    // Depth Buffer Setup

    glDisable(GL_DEPTH_TEST);                            // Disable the depth test for 2D

    glEnable(GL_BLEND);                                    // Enable Blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);                    // Type Of Blending To Perform
    // GEREK VAR MI? glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    // Really Nice Perspective Calculations
    // GEREK VAR MI? glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);                // Really Nice Point Smoothing
    glEnable(GL_TEXTURE_2D);                            // Enable Texture Mapping
    glBindTexture(GL_TEXTURE_2D, textures[0]);            // Select Our Texture

    

    for (loop = 0;loop < MAX_PARTICLES;loop++)                // Initials All The Textures
    {
        particle[loop].active = true;                                // Make All The Particles Active
        particle[loop].life = 1.0f;                                // Give All The Particles Full Life
        particle[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;    // Random Fade 
        particle[loop].r = vertex_colors[9][0];     // White color
        particle[loop].g = vertex_colors[9][1];
        particle[loop].b = vertex_colors[9][2];
        particle[loop].xi = float((rand() % 50) - 25.0f) * 10.0f;        // Random Speed On X Axis
        particle[loop].yi = float((rand() % 50) - 25.0f) * 10.0f;        // Random Speed On Y Axis
        particle[loop].zi = float((rand() % 50) - 25.0f) * 10.0f;        // Random Speed On Z Axis
        particle[loop].xg = 0.0f;                                    // Set Horizontal Pull
        particle[loop].yg = 0.8f;                                    // Set Vertical Pull
        particle[loop].zg = 0.0f;                                    // Set Pull On Z Axis To Zero
    }

    

}

// Reshape function
void G308_Reshape(int w, int h)
{
    if (h == 0) h = 1;

    g_nWinWidth = w;
    g_nWinHeight = h;

    glViewport(0, 0, g_nWinWidth, g_nWinHeight);
}

// Set Light
void SetLight()
{
    float direction[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    float diffintensity[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, direction);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffintensity);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

    glEnable(GL_LIGHT0);
}

// Set Camera Position
void SetCamera()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(G308_FOVY, (double)g_nWinWidth / (double)g_nWinHeight, G308_ZNEAR_3D, G308_ZFAR_3D);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(camX, camY, camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

}

void DrawGLScene()                                        // Here's Where We Do All The Drawing
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        // Clear Screen And Depth Buffer
    glLoadIdentity();                                        // Reset The ModelView Matrix
    for (loop = 0;loop < MAX_PARTICLES;loop++)                    // Loop Through All The Particles
    {
        if (particle[loop].active)                            // If The Particle Is Active
        {

            float x = particle[loop].x;                        // Grab Our Particle X Position
            float y = particle[loop].y;                        // Grab Our Particle Y Position
            float z = particle[loop].z + zoom;                    // Particle Z Pos + Zoom

            // Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
            glColor4f(particle[loop].r, particle[loop].g, particle[loop].b, particle[loop].life);

            glBegin(GL_TRIANGLE_STRIP);                        // Build Quad From A Triangle Strip
            glTexCoord2d(1, 1); glVertex3f(x + 0.5f, y + 0.5f, z); // Top Right
            glTexCoord2d(0, 1); glVertex3f(x - 0.5f, y + 0.5f, z); // Top Left
            glTexCoord2d(1, 0); glVertex3f(x + 0.5f, y - 0.5f, z); // Bottom Right
            glTexCoord2d(0, 0); glVertex3f(x - 0.5f, y - 0.5f, z); // Bottom Left
            glEnd();                                       // Done Building Triangle Strip

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

            particle[loop].xi += (particle[loop].xg + gravX);            // Take Pull On X Axis Into Account
            particle[loop].yi += (particle[loop].yg + gravY);            // Take Pull On Y Axis Into Account
            particle[loop].zi += (particle[loop].zg + gravZ);            // Take Pull On Z Axis Into Account
            particle[loop].life -= particle[loop].fade;        // Reduce Particles Life By 'Fade'

            if (particle[loop].life < 0.0f)                    // If Particle Is Burned Out
            {
                particle[loop].life = 1.0f;                    // Give It New Life
                particle[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;    // Random Fade Value
                particle[loop].x = posX;                        // Center On X Axis
                particle[loop].y = posY;                        // Center On Y Axis
                particle[loop].z = posZ;                        // Center On Z Axis
                particle[loop].xi = xspeed + float((rand() % 60) - 30.0f);    // X Axis Speed And Direction
                particle[loop].yi = yspeed + float((rand() % 60) - 30.0f);    // Y Axis Speed And Direction
                particle[loop].zi = float((rand() % 60) - 30.0f);    // Z Axis Speed And Direction
                particle[loop].r = vertex_colors[9][0];
                particle[loop].g = vertex_colors[9][1];
                particle[loop].b = vertex_colors[9][2];
            }
            else if (particle[loop].life < 0.4f)
            {
                // glUniform4f(particleColor, vertex_colors[8][0], vertex_colors[8][1], vertex_colors[8][2]);
                particle[loop].r = vertex_colors[8][0];		// Red color
    			particle[loop].g = vertex_colors[8][1];
    			particle[loop].b = vertex_colors[8][2];
            }
            else if (particle[loop].life < 0.6f)
            {
                particle[loop].r = orange[0];
                particle[loop].g = orange[1];
                particle[loop].b = orange[2];
            }
            else if (particle[loop].life < 0.75f)
            {
                particle[loop].r = yellow[0];
                particle[loop].g = yellow[1];
                particle[loop].b = yellow[2];
            }
            else if (particle[loop].life < 0.85f)
            {
                particle[loop].r = blue[0];
                particle[loop].g = blue[1];
                particle[loop].b = blue[2];
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
    glutInitWindowSize(g_nWinWidth, g_nWinHeight);
    g_mainWnd = glutCreateWindow("Fire Particles");
    
    /*glewExperimental = GL_TRUE;
    glewInit();*/
    InitGL();

    glutDisplayFunc(DrawGLScene);
    glutReshapeFunc(G308_Reshape);

    
    

    SetLight();
    SetCamera();

    glutMainLoop();

    return 0;
}
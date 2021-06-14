//
//  fireSmoke.cpp
//  FireSmokeProject
//
//  Created by Türkü Savran on 13.06.2021.
//

/*
*   HEADERS AND INCLUDES
*/
#define GL_SILENCE_DEPRECATION
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "define.h"
#include "Angel.h"

using namespace std;

// TypeDef
typedef vec3 point3;
typedef vec4 point4;
typedef vec4 color4;

// Number of Smoke Particles
const int fireParticleNum = 20000;
const int smokeParticleNum = 15000;

// Adjusting window width and height
GLuint windowWidth = 640;
GLuint windowHeight = 480;

//----------------------------------------------------------------------------

/*
*   STORING TEXTURES
*/

// Stores the fire particle bmp image at 0
// Stores the smoke particle bmp image at 1
GLuint textures[2];

// Both bmp images are 32 by 32
int textureWidth, textureHeight;
GLubyte* image;

//----------------------------------------------------------------------------

/*
*   PARTICLE PROPERTIES
*/
// 0: Loads the Fire bmp image, 1: Loads the Smoke bmp image
int textureType = 0;


//----------------------------------------------------------------------------

/*
*  PARTICLE STRUCTURE
*/

float xspeed;
float yspeed;
float zoom = -40.0f;

float posX = 0.0f;
float posY = -5.0f;
float posZ = 0.0f;

float gravX = 0.0f;
float gravY = 0.0f;
float gravZ = 0.0f;

float camX = 0.0f;
float camY = 2.5f;
float camZ = 50.0f;

// Used in particle manipulating for loop
GLuint loop;

typedef struct
{
    bool    active;     // Active (Yes/No)
    float    life;      // Particle Life
    float    fade;      // Fade Speed
    float    r;         // Red Value
    float    g;         // Green Value
    float    b;         // Blue Value
    float    x;          // X Position
    float    y;          // Y Position
    float    z;          // Z Position
    float    xi;         // X Direction
    float    yi;         // Y Direction
    float    zi;         // Z Direction
    float    xg;         // X Gravity
    float    yg;         // Y Gravity
    float    zg;         // Z Gravity
}
particles;

// Particle Array
particles particle[fireParticleNum];

//----------------------------------------------------------------------------

/*
*   LOADING TEXTURE
*/

void LoadTexture(const char* filename)
{
    unsigned char* data;

    FILE* file;
    file = fopen(filename, "rb");

    textureWidth = 32;
    textureHeight = 32;
    data = (unsigned char*)malloc(textureWidth * textureHeight * 3);
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
color4 vertex_colors[16] =
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
    color4(0.0, 1.0, 1.0, 1.0),			// Cyan
    color4(0.41, 0.41, 0.41, 1.0),	    // Dim-Grey
    color4(0.5, 0.5, 0.5, 1.0),			// Grey
    color4(0.6, 0.6, 0.6, 1.0),			// Dark Grey
    color4(0.82, 0.82, 0.82, 1.0),		// Light Grey
};

//----------------------------------------------------------------------------
/*
*   INIT FUNCTION
*/
// The function given is taken from the FireParticle.cpp file, and small edits have been made

void init()
{
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

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    
    // Initializing the particles and textures
    for (loop = 0; loop < fireParticleNum; loop++)
    {
        particle[loop].active = true;
        particle[loop].life = 1.0f;
        particle[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;
        particle[loop].r = vertex_colors[9][0];
        particle[loop].g = vertex_colors[9][1];
        particle[loop].b = vertex_colors[9][2];
        particle[loop].xi = float((rand() % 50) - 25.0f) * 10.0f;
        particle[loop].yi = float((rand() % 50) - 25.0f) * 10.0f;
        particle[loop].zi = float((rand() % 50) - 25.0f) * 10.0f;
        particle[loop].xg = 0.0f;
        particle[loop].yg = 0.8f;
        particle[loop].zg = 0.0f;
    }
}

//----------------------------------------------------------------------------
/*
*   RESHAPE FUNCTION
*/

void reshape(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, windowWidth, windowHeight);
}

//----------------------------------------------------------------------------
/*
*   CAMERA FUNCTION
*/
// The function given is taken from the FireParticle.cpp file (found in the references)

void SetCamera()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(G308_FOVY, (double) windowWidth / (double) windowHeight, G308_ZNEAR_3D, G308_ZFAR_3D);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(camX, camY, camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

//----------------------------------------------------------------------------
/*
*   DRAWING THE PARTICLES
*/
// The functions given are taken from the FireParticle.cpp file, and small edits have been made

void drawFireParticle() {
    // Loop Through All The Particles
    for (loop = 0; loop < fireParticleNum; loop++)
    {
        // Check if particle is active
        if (particle[loop].active)
        {

            float x = particle[loop].x;
            float y = particle[loop].y;
            float z = particle[loop].z + zoom;

            // Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
            glColor4f(particle[loop].r, particle[loop].g, particle[loop].b, particle[loop].life);

            glBegin(GL_TRIANGLES);
            glTexCoord2d(1, 1); glVertex3f(x + 0.5f, y + 0.5f, z);
            glTexCoord2d(0, 1); glVertex3f(x - 0.5f, y + 0.5f, z);
            glTexCoord2d(1, 0); glVertex3f(x + 0.5f, y - 0.5f, z);
            glTexCoord2d(0, 0); glVertex3f(x - 0.5f, y - 0.5f, z);
            glEnd();

            particle[loop].x += particle[loop].xi / (750);
            particle[loop].y += particle[loop].yi / (1000);
            particle[loop].z += particle[loop].zi / (750);

            if ((particle[loop].x > posX) && (particle[loop].y > (0.1 + posY))) {
                particle[loop].xg = -0.3f;
            }
            else if ((particle[loop].x < posX) && (particle[loop].y > (0.1 + posY))) {
                particle[loop].xg = 0.3f;
            }
            else {
                particle[loop].xg = 0.0f;
            }

            particle[loop].xi += (particle[loop].xg + gravX);
            particle[loop].yi += (particle[loop].yg + gravY);
            particle[loop].zi += (particle[loop].zg + gravZ);
            particle[loop].life -= particle[loop].fade;

            if (particle[loop].life < 0.0f)
            {
                particle[loop].life = 1.0f;
                particle[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;
                particle[loop].x = posX;
                particle[loop].y = posY;
                particle[loop].z = posZ;
                particle[loop].xi = xspeed + float((rand() % 60) - 30.0f);
                particle[loop].yi = yspeed + float((rand() % 60) - 30.0f);
                particle[loop].zi = float((rand() % 60) - 30.0f);
                particle[loop].r = vertex_colors[9][0];
                particle[loop].g = vertex_colors[9][1];
                particle[loop].b = vertex_colors[9][2];
            }
            else if (particle[loop].life < 0.4f)
            {
                particle[loop].r = vertex_colors[8][0];
                particle[loop].g = vertex_colors[8][1];
                particle[loop].b = vertex_colors[8][2];
            }
            else if (particle[loop].life < 0.6f)
            {
                particle[loop].r = vertex_colors[7][0];
                particle[loop].g = vertex_colors[7][1];
                particle[loop].b = vertex_colors[7][2];
            }
            else if (particle[loop].life < 0.75f)
            {
                particle[loop].r = vertex_colors[6][0];
                particle[loop].g = vertex_colors[6][1];
                particle[loop].b = vertex_colors[6][2];
            }
            else if (particle[loop].life < 0.85f)
            {
                particle[loop].r = vertex_colors[4][0];
                particle[loop].g = vertex_colors[4][1];
                particle[loop].b = vertex_colors[4][2];
            }
        }
    }
}

void drawSmokeParticle() {
    for (loop = 0; loop < smokeParticleNum; loop++)
    {
        if (particle[loop].active)
        {

            float x = particle[loop].x;
            float y = particle[loop].y;
            float z = particle[loop].z + zoom;

            // Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
            glColor4f(particle[loop].r, particle[loop].g, particle[loop].b, particle[loop].life);

            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2d(1, 1); glVertex3f(x + 0.5f, y + 0.5f, z);
            glTexCoord2d(0, 1); glVertex3f(x - 0.5f, y + 0.5f, z);
            glTexCoord2d(1, 0); glVertex3f(x + 0.5f, y - 0.5f, z);
            glTexCoord2d(0, 0); glVertex3f(x - 0.5f, y - 0.5f, z);
            glEnd();

            particle[loop].x += particle[loop].xi / (750);
            particle[loop].y += particle[loop].yi / (1000);
            particle[loop].z += particle[loop].zi / (750);

            if ((particle[loop].x > posX) && (particle[loop].y > (0.1 + posY))) {
                particle[loop].xg = -0.3f;
            }
            else if ((particle[loop].x < posX) && (particle[loop].y > (0.1 + posY))) {
                particle[loop].xg = 0.3f;
            }
            else {
                particle[loop].xg = 0.0f;
            }

            particle[loop].xi += (particle[loop].xg + gravX);
            particle[loop].yi += (particle[loop].yg + gravY);
            particle[loop].zi += (particle[loop].zg + gravZ);
            particle[loop].life -= particle[loop].fade;

            if (particle[loop].life < 0.0f)
            {
                particle[loop].life = 1.0f;
                particle[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;
                particle[loop].x = posX;
                particle[loop].y = posY;
                particle[loop].z = posZ;
                particle[loop].xi = xspeed + float((rand() % 60) - 30.0f);
                particle[loop].yi = yspeed + float((rand() % 20) - 10.0f);
                particle[loop].zi = float((rand() % 60) - 30.0f);

                // Dim-Grey
                particle[loop].r = vertex_colors[12][0];
                particle[loop].g = vertex_colors[12][1];
                particle[loop].b = vertex_colors[12][2];
            }
            else if (particle[loop].life < 0.4f)
            {
                // Light Grey
                particle[loop].r = vertex_colors[15][0];
                particle[loop].g = vertex_colors[15][1];
                particle[loop].b = vertex_colors[15][2];
            }
            else if (particle[loop].life < 0.6f)
            {
                // Grey
                particle[loop].r = vertex_colors[13][0];
                particle[loop].g = vertex_colors[13][1];
                particle[loop].b = vertex_colors[13][2];
            }
            else if (particle[loop].life < 0.75f)
            {
                // Grey
                particle[loop].r = vertex_colors[13][0];
                particle[loop].g = vertex_colors[13][1];
                particle[loop].b = vertex_colors[13][2];
            }
            else if (particle[loop].life < 0.85f)
            {
                // Dark Grey
                particle[loop].r = vertex_colors[14][0];
                particle[loop].g = vertex_colors[14][1];
                particle[loop].b = vertex_colors[14][2];
            }
        }
    }
}

//----------------------------------------------------------------------------
/*
*   DISPLAY FUNCTION
*/

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Draw fire texture if texture type is 0, otherwise draw smoke texture
    if (textureType == 0) {
        drawFireParticle();
    }
    else {
        drawSmokeParticle();
    }
    glutPostRedisplay();
    glutSwapBuffers();
}

//----------------------------------------------------------------------------
/*
*   MAIN MENU
*/

void mainMenuEvents(int selection)
{
    switch (selection)
    {
    case 0:
        cout << "Selected the fire particle." << endl;
        textureType = 0;
        glBindTexture(GL_TEXTURE_2D, textures[0]);      // Binds the fire particle image
        break;

    case 1:
        cout << "Selected the smoke particle." << endl;
        textureType = 1;
        glBindTexture(GL_TEXTURE_2D, textures[1]);      // Binds the fire particle image
        break;
    }
}

void createMainMenu() {
    int mainMenu;

    // Creating the main menu
    mainMenu = glutCreateMenu(mainMenuEvents);
    glutAddMenuEntry("Fire", 0);
    glutAddMenuEntry("Smoke", 1);

    // Attach the main menu to the right button
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//----------------------------------------------------------------------------

/*
*   TIMER
*/

void timer(int p)
{
    glutPostRedisplay();
    glutTimerFunc(20, timer, 0);
}

//----------------------------------------------------------------------------
/*
*   KEYBOARD FUNCTION
*/

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'q': case 'Q':
        exit(0);
    case 'h': case 'H':
        std::cout << "Fire & Smoke Particle Project" << std::endl;
        std::cout << " - Press Q to exit." << std::endl;
        std::cout << " - Press H for help." << std::endl;
        std::cout << " - Left click to reach the menu." << std::endl;
        break;
    }
}

//----------------------------------------------------------------------------
/*
*   MAIN
*/

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Fire & Smoke Particles");
    
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    createMainMenu();
    SetCamera();
    glutTimerFunc(20, timer, 0);

    glutMainLoop();

    return 0;
}
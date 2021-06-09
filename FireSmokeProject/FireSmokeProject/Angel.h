//////////////////////////////////////////////////////////////////////////////
//
//  --- Angel.h ---
//
//   The main header file for all examples from Angel 6th Edition
//
//////////////////////////////////////////////////////////////////////////////

// Some modifications for system compatibility made by Ben Wiley 2017

#ifndef __ANGEL_H__
#define __ANGEL_H__

//----------------------------------------------------------------------------

#include <cmath>
#include <iostream>

//  Define M_PI in the case it's not defined in the math header file
#ifndef M_PI
#  define M_PI  3.14159265358979323846
#endif

//----------------------------------------------------------------------------
//
// --- Include OpenGL header files and helpers ---
//
//   The location of these files vary by operating system.  We've included
//     copies of open-soruce project headers in the "GL" directory local
//     this this "include" directory.
//
//
#ifdef __APPLE__  // include Mac OS X verions of headers
#  include <OpenGL/gl3.h>
#  include <OpenGL/gl3ext.h>
#  define __gl_h_ // don't include gl.h: https://stackoverflow.com/a/24098402
#  include <GLUT/glut.h>
#  include <OpenGL/OpenGL.h>
#else // non-Mac OS X operating systems
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <GL/freeglut_ext.h>
#endif

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

//----------------------------------------------------------------------------
//
//  --- Include our class libraries and constants ---
//

namespace Angel {

GLuint InitShader( const char* vertexShaderFile,
		   const char* fragmentShaderFile );

const GLfloat  DivideByZeroTolerance = GLfloat(1.0e-07);

//  Degrees-to-radians constant 
const GLfloat  DegreesToRadians = M_PI / 180.0;

}  // namespace Angel

#include "vec.h"
#include "mat.h"
//#include "CheckError.h"

//  Globally use our namespace in our example programs.
using namespace Angel;

#endif // __ANGEL_H__

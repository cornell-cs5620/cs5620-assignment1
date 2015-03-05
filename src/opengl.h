#ifndef opengl_h
#define opengl_h

#if defined(OSX_CORE3) || defined(OSX_CORE2) || defined(__APPLE__)
    #include <GLUT/glut.h>
#else
    #include <GL/glew.h>
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glext.h>
#endif

#include "standard.h"

GLuint load_shader_file(string filename, GLenum type);
GLuint load_shader_source(string source, GLenum type);

#endif

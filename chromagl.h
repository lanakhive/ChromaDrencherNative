#ifndef CHROMAGL_H
#define CHROMAGL_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "chroma.h"

GLuint makeShader(const char *source, GLenum type);

// attribute and uniform location defines
#define ATB_POSITION 0
#define ATB_ACOLOR 1
#define ATB_OFFSET 2
#define UNI_MVM 0
#define UNI_PM 1
#define UNI_LINESIZE 2
#define UNI_TIPFRAC 3
#define UNI_RESOLUTION 4
#define UNI_ABLEND 5
#define UNI_ALEN1 6
#define UNI_ALEN2 7
#define UNI_ACOLOR1 8
#define UNI_ACOLOR2 13

extern const char *vxshad;
extern const char *fgshad;

extern float ver[];
extern float col[];

extern float *lineBuffer;
extern int lineBufferCount;

extern GLuint program1;
extern GLuint vao;
extern GLuint bufferOffset;

void initChromaGL();

void drawChromaGL();

void serializeLineList();

#endif // CHROMAGL_H

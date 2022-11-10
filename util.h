#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

void blockAlloc(int newCount, int *oldCount, int itemSize, void **buffer);

GLuint makeShader(const char *source, GLenum type);

void rgb2hsv(float c[3]);

#endif //UTIL_H
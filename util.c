#include "util.h"
//#include "SDL2\SDL.h"

void blockAlloc(int newCount, int *oldCount, int itemSize, void **buffer) {
	//if (*oldCount > 0) free(*buffer);
	const unsigned int block = 4096;
	int oldItems = ceil(*oldCount/(float)block)*block;
	int newItems = ceil(newCount/(float)block)*block;
	int allocBytes = itemSize*newItems;
	//SDL_Log("Alloc: %d item, %d bytes", newItems, allocBytes);
	//*buffer = malloc(allocBytes);
	if (oldItems != newItems || *oldCount == 0) {
		//SDL_Log("realloc %d", allocBytes);
		void *newbuffer = realloc(*buffer, allocBytes);
		if (newbuffer == NULL) {
			//SDL_Log("Alloc Failed.");
			exit(1);
		} else {
			if (*buffer != newbuffer) {
				//SDL_Log("Ptr change");
			}
			*buffer = newbuffer;
		}
	}
	*oldCount = newItems;
}

GLuint makeShader(const char *source, GLenum type) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) {
		//SDL_Log("Shader compile successful.");
		return shader;
	} else {
		//GLint maxLength = 0;
		//glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
		char buf[513];
		glGetShaderInfoLog(shader, 512, NULL, buf);
		//SDL_Log("Shader compile failed:\n%s", buf);
		glDeleteShader(shader);
		exit(1);
	}
	return 0;
}

float min3(float a, float b, float c) {
	if (a <= b && a <= c) return a;
	else if (b <= a && b <= c) return b;
	else if (c <= a && c <= b) return c;
}

float max3(float a, float b, float c) {
	if (a >= b && a >= c) return a;
	else if (b >= a && b >= c) return b;
	else if (c >= a && c >= b) return c;
}

void rgb2hsv(float c[3])
{
	float h, s, v;
	float min = min3(c[0], c[1], c[2]);
	float max = max3(c[0], c[1], c[2]);
	float delta = max - min;
	v = max / 255.0;
	if (max == 0) {
		h = 0;
		s = 0;
	} else {
		s = delta / max;
		if (c[0] == max) h = (c[1] - c[2]) / delta;
		if (c[1] == max) h = 2 + (c[2] - c[0]) / delta;
		if (c[2] == max) h = 4 + (c[0] - c[1]) / delta;
		//h *= 60.0;
		//if (h < 0) h += 360.0;
		//if (h > 360.0) h -= 360.0;
		if (h < 0) h += 6.0;
		if (h > 6) h -= 6.0;
	}
	c[0] = h;
	c[1] = s;
	c[2] = v;
}
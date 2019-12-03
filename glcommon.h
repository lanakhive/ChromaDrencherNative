#include "chroma.h"
#include "chromagl.h"
#include "gl-matrix.h"

volatile int screenWidth = 1024;
volatile int screenHeight = 768;

float identityMatrix[] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

float projectionMatrix[16] = {0.0};

void startupCommon() {
	// seed random numbers
	srand(time(NULL));

	// init chroma lines
	prepareLines(screenWidth, screenHeight);

	// generate inital color gradient sets
	coolColors(true);
	coolColors(false);

	// setup camera
	//mat4_frustum(0, 40, 40, 0, 10, 200, perp);
	mat4_ortho(0, screenWidth, screenHeight, 0, 0, 200, projectionMatrix);
	//mat4_rotateZ(perp, -M_PI, NULL);
	//float transs[] = {-screenWidth, -screenHeight, 0};
	//mat4_translate(perp, transs, NULL);
}

void init() {
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	// set initial gl feature state
	glViewport(0, 0, screenWidth, screenHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_MULTISAMPLE);

	// clear the framebuffer before starting rendering
	glClear(GL_COLOR_BUFFER_BIT);

	// prepare shaders and buffers
	initChromaGL();
}

void draw() {
	glViewport(0, 0, screenWidth, screenHeight);

	//drawChromaGL(screenWidth, screenHeight, orthographic, matrixFlip);
	drawChromaGL(screenWidth, screenHeight, projectionMatrix, identityMatrix);
}

void resize(int newScreenWidth, int newScreenHeight) {
	// regenerate the orthographic projection matrix
	mat4_ortho(0, newScreenWidth, newScreenHeight, 0, 0, 200, projectionMatrix);

	// restart chroma drencher
	prepareLines(newScreenWidth, newScreenHeight);
}

void update(float dt) {
	updateLines(dt, screenWidth, screenHeight);
}
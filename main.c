#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "gl-matrix.h"
#include "chroma.h"
#include "chromagl.h"

int screenWidth = 1024;
int screenHeight = 768;

SDL_Window *window;
SDL_GLContext glcontext;
SDL_Thread *renderThread;
volatile bool quitApp = false;
volatile bool running = true;
volatile bool shouldSwitchVsync = false;
volatile bool shouldResize = false;
SDL_mutex *mutex;

float identityMatrix[] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

float projectionMatrix[16] = {0.0};


void oglDebugError(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char *message, const void *userparam) {
	// opengl error from ARB_DEBUG_OUTPUT
	SDL_Log("OpenGL Error: %s", message);
}

void toggleSwapInterval() {
	int success;
	int current = SDL_GL_GetSwapInterval();
	switch (current) {
		// vsync off -> enable vsync
		case 0:
			success = SDL_GL_SetSwapInterval(1);
			if (success == -1) {
				success = SDL_GL_SetSwapInterval(0);
				SDL_Log("Using Immediate Sync");
				break;
			} else {
				SDL_Log("Using Vertical Sync");
			}
			break;
		// vsync on -> enable adaptive vsync
		case 1:
			success = SDL_GL_SetSwapInterval(-1);
			if (success == -1) {
				success = SDL_GL_SetSwapInterval(0);
				SDL_Log("Using Immediate Sync");
				break;
			} else {
				SDL_Log("Using Adaptive Vertical Sync");
			}
			break;
		// adaptive vsync on -> disable vsync
		case -1:
			SDL_GL_SetSwapInterval(0);
			SDL_Log("Using Immediate Sync");
			break;
	}
}

void toggleFullscreen() {
	int windowFlags = SDL_GetWindowFlags(window);
	if (windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
		// fullscreen on -> switch off
		SDL_SetWindowFullscreen(window, 0);
		SDL_ShowCursor(SDL_ENABLE);
	} else {
		// fullscreen off -> switch on
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_ShowCursor(SDL_DISABLE);
	}
}


void initGL() {

	// create openGL context
	glcontext = SDL_GL_CreateContext(window);
	if (glcontext == NULL) {SDL_Log("GL context fail: %s", SDL_GetError()); exit(1);}
	SDL_Log("OpenGL %s\n", glGetString(GL_VERSION));
	SDL_Log("GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	SDL_Log("Renderer %s\n", glGetString(GL_RENDERER));

	// load openGL func pointers
	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK) {
		SDL_Log("GLEW failed: %s", glewGetErrorString(glewStatus));
		exit(1);
	}
	SDL_Log("GLEW %s\n", glewGetString(GLEW_VERSION));

	// try to enable adaptive vsync or normal vsync
	int swapSuccess = SDL_GL_SetSwapInterval(-1);
	if (swapSuccess == -1) SDL_GL_SetSwapInterval(1);

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	// set initial gl feature state
	glViewport(0, 0, screenWidth, screenHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_MULTISAMPLE);

	// clear the framebuffer before showing the window
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);
	SDL_ShowWindow(window);
	SDL_RaiseWindow(window);

#ifdef DEBUG
	glDebugMessageCallbackARB(oglDebugError, NULL);
#endif

	initChromaGL();

}

void draw(int screenWidth, int screenHeight) {

	glViewport(0, 0, screenWidth, screenHeight);

	//drawChromaGL(screenWidth, screenHeight, orthographic, matrixFlip);
	drawChromaGL(screenWidth, screenHeight, projectionMatrix, identityMatrix);

	SDL_GL_SwapWindow(window);
}

int renderMain(void *data) {
	//SDL_Log("renderMain start");
	initGL();
	int ctx=0;
	unsigned int lastTime = 0, currentTime;
	float dt = 0;
	for (;;) {

		// calculate variable time step
		currentTime = SDL_GetTicks();
		dt = (currentTime - lastTime) / (1000.0);
		lastTime = currentTime;
		if (dt > 1.0) dt = 0.1;

		if (running) {
			//SDL_Log("Draw Frame %d", ctx++);
			updateLines(dt, screenWidth, screenHeight);
			draw(screenWidth, screenHeight);
		} else {
			//SDL_Log("Sleep Frame %d", ctx++);
			SDL_Delay(100);
		}
		if (shouldSwitchVsync) {
			shouldSwitchVsync = false;
			toggleSwapInterval();
		}

		SDL_LockMutex(mutex);
		if (shouldResize) {
			shouldResize = false;
			//SDL_Log("------ to %dx%d", screenWidth, screenHeight);
			mat4_ortho(0, screenWidth, screenHeight, 0, 0, 200, projectionMatrix);
			prepareLines(screenWidth, screenHeight);
			draw(screenWidth, screenHeight);
		}
		SDL_UnlockMutex(mutex);


		if (quitApp) {
			//SDL_Log("renderMain quit");
			SDL_GL_DeleteContext(glcontext);
			return 0;
		}
	}
}

void resize(int width, int height) {
	SDL_LockMutex(mutex);
	screenWidth = width;
	screenHeight = height; 
	shouldResize = true;
	//SDL_Log("Resize to %dx%d", screenWidth, screenHeight);
	SDL_UnlockMutex(mutex);
}

int eventFilter(void *userdata, SDL_Event *event) {
	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
		//screenWidth = event->window.data1;
		//screenHeight = event->window.data2;
		resize(event->window.data1, event->window.data2);
		return 0;
	}
	return 1;
}

void handleEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				quitApp = true;
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					//bypassed by event filter
					//resize(event.window.data1, event.window.data2);
				}
				if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				}
				break;
			case SDL_KEYDOWN:
				SDL_Log("Key sym %d", event.key.keysym.sym);
				switch (event.key.keysym.sym) {
					case SDLK_f:
						toggleFullscreen();
						break;
					case SDLK_s:
						shouldSwitchVsync = true;
						break;
					case SDLK_p:
						running = !running;
						break;
					case SDLK_ESCAPE:
						quitApp = true;
						break;
				}
				break;
		}
	}
}

void startup()
{
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

	SDL_Log("Chroma Drencher 1.0");

	SDL_version sdlVersion;
	SDL_GetVersion(&sdlVersion);
	SDL_Log("SDL %d.%d.%d", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);

	// init SDL
	int initStatus = SDL_Init(SDL_INIT_VIDEO);
	if (initStatus != 0) {SDL_Log("SDL Init failed: %s", SDL_GetError()); exit(1);}
	atexit(SDL_Quit);

	SDL_SetEventFilter(eventFilter, NULL);

	// select openGL context version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// create window
	const char *windowName = "Chroma Drencher";
	window = SDL_CreateWindow(windowName, 800, 100, screenWidth, screenHeight,
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIDDEN);

	mutex = SDL_CreateMutex();
	renderThread = SDL_CreateThread(renderMain, "RenderThread", NULL);
}

void run()
{
	// main loop
	while(!quitApp)
	{
		// handle events and sleep when done
		SDL_Delay(1);
		handleEvents();
	}
	int renderThreadReturnVal;
	SDL_WaitThread(renderThread, &renderThreadReturnVal);
	//SDL_Log("Main quit");
}

void shutdown() {
	SDL_DestroyWindow(window);
	SDL_DestroyMutex(mutex);
}

int main(int argc, char **argv)
{
	startup();
	run();
	shutdown();
	return 0;
}



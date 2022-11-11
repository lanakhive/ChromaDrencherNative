#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glcommon.h"

SDL_Window *mainWindowSDL;
SDL_GLContext glContextSDL;
SDL_Thread *appThreadSDL;

volatile bool running = true;
volatile bool quitApp = false;
volatile bool shouldSwitchVsync = false;
volatile bool shouldResize = false;

void oglDebugMessage(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char *message, const void *userparam) {
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
	int windowFlags = SDL_GetWindowFlags(mainWindowSDL);
	if (windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
		// fullscreen on -> switch off
		SDL_SetWindowFullscreen(mainWindowSDL, 0);
		SDL_ShowCursor(SDL_ENABLE);
	} else {
		// fullscreen off -> switch on
		SDL_SetWindowFullscreen(mainWindowSDL, SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_ShowCursor(SDL_DISABLE);
	}
}


void initGL() {

	// create openGL context
	glContextSDL = SDL_GL_CreateContext(mainWindowSDL);
	if (glContextSDL == NULL) {SDL_Log("GL context fail: %s", SDL_GetError()); exit(1);}
	SDL_Log("OpenGL %s\n", glGetString(GL_VERSION));
	SDL_Log("GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	SDL_Log("Renderer %s\n", glGetString(GL_RENDERER));

	// load openGL function pointers
	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK) {
		SDL_Log("GLEW failed: %s", glewGetErrorString(glewStatus));
		exit(1);
	}
	SDL_Log("GLEW %s\n", glewGetString(GLEW_VERSION));

#ifdef DEBUG
	glDebugMessageCallbackARB(oglDebugMessage, NULL);
#endif

	// try to enable adaptive vsync or normal vsync
	int swapSuccess = SDL_GL_SetSwapInterval(-1);
	if (swapSuccess == -1) SDL_GL_SetSwapInterval(1);

	init();

	SDL_GL_SwapWindow(mainWindowSDL);
	SDL_ShowWindow(mainWindowSDL);
	SDL_RaiseWindow(mainWindowSDL);

}

int AppThreadMainSDL(void *data) {
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
			update(dt);
			draw();
		}
		if (shouldSwitchVsync) {
			shouldSwitchVsync = false;
			toggleSwapInterval();
		}

		if (shouldResize) {
			shouldResize = false;
			//SDL_Log("------ to %dx%d", screenWidth, screenHeight);
			resize(screenWidth, screenHeight);
			draw();
		}

		if (quitApp) {
			//SDL_Log("renderMain quit");
			SDL_GL_DeleteContext(glContextSDL);
			return 0;
		}

		SDL_GL_SwapWindow(mainWindowSDL);

	}
}

int eventFilter(void *userdata, SDL_Event *event) {
	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
		screenWidth = event->window.data1;
		screenHeight = event->window.data2;
		shouldResize = true;
		while (shouldResize);
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
	startupCommon();

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
	mainWindowSDL = SDL_CreateWindow(windowName, 800, 100, screenWidth, screenHeight,
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIDDEN);

	appThreadSDL = SDL_CreateThread(AppThreadMainSDL, "ApplicationThread", NULL);
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
	int appThreadReturnVal;
	SDL_WaitThread(appThreadSDL, &appThreadReturnVal);
	//SDL_Log("Main quit");
}

void shutdown() {
	SDL_DestroyWindow(mainWindowSDL);
}

int main(int argc, char **argv)
{
	startup();
	run();
	shutdown();
	return 0;
}



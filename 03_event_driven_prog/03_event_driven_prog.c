#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

int quit = 0;
SDL_Event e;

#define PATH_TO_BMP "../medias/test.bmp"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window *window;
SDL_Surface *surface;
SDL_Surface *hello_world;

// init SDL, create a window and get its surface
int init()
{
	int ret;

	// init SDL
	ret = SDL_Init(SDL_INIT_VIDEO);
	if (ret < 0) {
		printf("SDL_Init ERROR: %s\n", SDL_GetError());
		return ret;
	}

	// create window
	window = SDL_CreateWindow("SDL_tuto_01", SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
				  SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		printf("SDL_SetVideoMode ERROR: %s\n", SDL_GetError());
		return -1;
	}

	// get window surface
	surface = SDL_GetWindowSurface(window);
	if (!surface) {
		printf("SDL_GetWindowSurface ERROR: %s\n", SDL_GetError());
		return -1;
	}

	return 0;
}

// load picture on screen
int load_media()
{
	// load image
	hello_world = SDL_LoadBMP(PATH_TO_BMP);
	if (!hello_world) {
		printf("unable to load picture '%s'\n", PATH_TO_BMP);
		printf("SDL_GetWindowSurface ERROR: %s\n", SDL_GetError());
		return -1;
	}
	return 0;
}

// free and close stuff
void leave()
{
	//free surface
	SDL_FreeSurface(hello_world);
	hello_world = NULL;

	//destroy window
	SDL_DestroyWindow(window);
	window = NULL;

	//quit SDL
	SDL_Quit();
}

int main()
{
	init();
	load_media();

	//While application is running
	while (!quit) {
		// handle events
		while (SDL_PollEvent(&e) != 0) {
			//user ask to quit
			if (e.type == SDL_QUIT ||
			    (e.type == SDL_KEYDOWN &&
			     e.key.keysym.sym == SDLK_ESCAPE)) {
				quit = 1;
			}
		}

		// apply the image
		SDL_BlitSurface(hello_world, NULL, surface, NULL);

		// update surface
		SDL_UpdateWindowSurface(window);
	}

	leave();

	return 0;
}
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main()
{
	SDL_Window *window;
	SDL_Surface *surface;
	int ret;

	// init SDL
	ret = SDL_Init(SDL_INIT_VIDEO);
	if (ret < 0) {
		printf("SDL_Init ERROR: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	// create window
	window = SDL_CreateWindow("SDL_tuto_01", SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
				  SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		printf("SDL_SetVideoMode ERROR: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	// get window surface
	surface = SDL_GetWindowSurface(window); 
	if (!surface) {
		printf("SDL_GetWindowSurface ERROR: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	// Fill the surface white
	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xff, 0xFF, 0xff));

	// update surface
	SDL_UpdateWindowSurface(window);

	// wait 3 seconds
	SDL_Delay(3000);

	// Destroy window
	SDL_DestroyWindow(window);

	// Quit SDL
	SDL_Quit();

	return 0;
}
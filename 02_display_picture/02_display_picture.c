#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

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

// the main function...
int main()
{
	int ret = 0;

    // init
	ret = init();
	if (ret < 0) {
		printf("init FAILED\n");
		return 0;
	}

    // load pict
	ret = load_media();
	if (ret < 0) {
		printf("load_media FAILED\n");
		return 0;
	}

    // display pict
    SDL_BlitSurface(hello_world, NULL, surface, NULL);
    SDL_UpdateWindowSurface(window);
    SDL_Delay(3000);

    // exit
    leave();

	return 0;
}
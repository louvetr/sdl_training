//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>


#define PATH_TO_PNG "./test.png"

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 1024;

SDL_Window *window;
SDL_Surface *screen_surface;
SDL_Surface *rick;

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
	window = SDL_CreateWindow("SDL_tuto", SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
				  SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		printf("SDL_SetVideoMode ERROR: %s\n", SDL_GetError());
		return -1;
	}

    int img_flags = IMG_INIT_PNG;
    if( !(IMG_Init(img_flags) & img_flags)) {
  		printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
		return -1;      
    }

	// get window surface
	screen_surface = SDL_GetWindowSurface(window);
	if (!screen_surface) {
		printf("SDL_GetWindowSurface ERROR: %s\n", SDL_GetError());
		return -1;
	}

	return 0;
}


SDL_Surface *load_surface(char *path)
{
	SDL_Surface *opt_surface = NULL;

	SDL_Surface *loaded_surface = IMG_Load(path);
	if (!loaded_surface) {
		printf("Unable to load image %s! SDL Error: %s\n", path,
		       SDL_GetError());
		return NULL;
	}

	opt_surface =
		SDL_ConvertSurface(loaded_surface, screen_surface->format, 0);
	if (!opt_surface)
		printf("Unable to optimize image %s! SDL Error: %s\n", path,
		       SDL_GetError());
}


// free and close stuff
void leave()
{
	//free surface
	SDL_FreeSurface(rick);
	rick = NULL;

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

	// display pict
	rick = load_surface(PATH_TO_PNG);
	SDL_Rect stretched_rect;
	stretched_rect.x = 0;
	stretched_rect.y = 0;
	stretched_rect.w = SCREEN_WIDTH;
	stretched_rect.h = SCREEN_HEIGHT;

	SDL_BlitScaled(rick, NULL, screen_surface, &stretched_rect);
	SDL_UpdateWindowSurface(window);
	SDL_Delay(3000);

	// exit
	leave();

	return 0;
}
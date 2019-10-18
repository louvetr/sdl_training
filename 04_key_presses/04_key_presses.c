#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>


//Key press surfaces constants
enum KeyPressSurfaces {
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_TOTAL
};


const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 240;

SDL_Window *window;
SDL_Surface *screen_surface;
SDL_Surface *current_surface;
SDL_Surface *key_press_surfaces[KEY_PRESS_SURFACE_TOTAL];


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
	screen_surface = SDL_GetWindowSurface(window);
	if (!screen_surface) {
		printf("SDL_GetWindowSurface ERROR: %s\n", SDL_GetError());
		return -1;
	}

	return 0;
}

SDL_Surface *load_surface(char *path)
{
	//Load image at specified path
	SDL_Surface *loaded_surface = SDL_LoadBMP(path);
	if (loaded_surface == NULL) {
		printf("Unable to load image %s! SDL Error: %s\n", path,
		       SDL_GetError());
	}

	return loaded_surface;
}

// load picture on screen
int load_media()
{
	int ret;

	// default surface
	key_press_surfaces[KEY_PRESS_SURFACE_DEFAULT] =
		load_surface("../medias/press.bmp");
	if (!key_press_surfaces[KEY_PRESS_SURFACE_DEFAULT]) {
		printf("Failed to load default image!\n");
		ret = -1;
	}

	// up surface
	key_press_surfaces[KEY_PRESS_SURFACE_UP] = load_surface("../medias/up.bmp");
	if (!key_press_surfaces[KEY_PRESS_SURFACE_UP]) {
		printf("Failed to load up image!\n");
		ret = -1;
	}

	// down surface
	key_press_surfaces[KEY_PRESS_SURFACE_DOWN] = load_surface("../medias/down.bmp");
	if (!key_press_surfaces[KEY_PRESS_SURFACE_DOWN]) {
		printf("Failed to load down image!\n");
		ret = -1;
	}

	// left surface
	key_press_surfaces[KEY_PRESS_SURFACE_LEFT] = load_surface("../medias/left.bmp");
	if (!key_press_surfaces[KEY_PRESS_SURFACE_LEFT]) {
		printf("Failed to load left image!\n");
		ret = -1;
	}

	// right surface
	key_press_surfaces[KEY_PRESS_SURFACE_RIGHT] = load_surface("../medias/right.bmp");
	if (!key_press_surfaces[KEY_PRESS_SURFACE_RIGHT]) {
		printf("Failed to load right image!\n");
		ret = -1;
	}

	return 0;
}

// free and close stuff
void leave()
{
	//free surface
	SDL_FreeSurface(screen_surface);
	screen_surface = NULL;

	//destroy window
	SDL_DestroyWindow(window);
	window = NULL;

	//quit SDL
	SDL_Quit();
}

int main()
{
	int quit = 0;
	SDL_Event e;
	current_surface = key_press_surfaces[KEY_PRESS_SURFACE_DEFAULT];

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

			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_UP:
					current_surface = key_press_surfaces[KEY_PRESS_SURFACE_UP];
					break;
				case SDLK_DOWN:
					current_surface = key_press_surfaces[KEY_PRESS_SURFACE_DOWN];
					break;
				case SDLK_LEFT:
					current_surface = key_press_surfaces[KEY_PRESS_SURFACE_LEFT];
					break;
				case SDLK_RIGHT:
					current_surface = key_press_surfaces[KEY_PRESS_SURFACE_RIGHT];
					break;
				default:
					current_surface = key_press_surfaces[KEY_PRESS_SURFACE_DEFAULT];
					break;
				}
			}
		}

		// apply the image
		SDL_BlitSurface(current_surface, NULL, screen_surface, NULL);

		// update surface
		SDL_UpdateWindowSurface(window);
	}

	leave();

	return 0;
}

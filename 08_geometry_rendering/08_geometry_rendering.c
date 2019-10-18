//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *surface;

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

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		printf("Renderer could not be created! SDL Error: %s\n",
		       SDL_GetError());
		return -1;
	}

	SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0xFF, 0x00);

	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
		       IMG_GetError());
		return -1;
	}

	return 0;
}

int load_media()
{
	return 0;
}

// free and close stuff
void leave()
{
	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	//destroy window
	SDL_DestroyWindow(window);
	window = NULL;

	//quit SDL
	IMG_Quit();
	SDL_Quit();
}

int main()
{
	int quit = 0;
	SDL_Event e;

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

		// clear screen
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		// render red filled quad
		SDL_Rect fill_rect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4,
				       SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
		SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
		SDL_RenderFillRect(renderer, &fill_rect);

		// render green empty quad
		SDL_Rect line_rect = { SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6,
				       SCREEN_WIDTH * 2 / 3,
				       SCREEN_HEIGHT * 2 / 3 };
		SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
		SDL_RenderDrawRect(renderer, &line_rect);

		// draw blue line horizon
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
		SDL_RenderDrawLine(renderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH,
				   SCREEN_HEIGHT / 2);

		// draw vertical line of yellow dots
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
		for (int i = 0; i < SCREEN_HEIGHT; i += 4)
			SDL_RenderDrawPoint(renderer, SCREEN_WIDTH / 2, i);

		// update screen
		SDL_RenderPresent(renderer);
	}

	leave();

	return 0;
}

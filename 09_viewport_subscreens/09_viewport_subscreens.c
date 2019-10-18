//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define PATH_TO_PNG "../medias/test.png"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Surface *surface;

char resolved_path[128];

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

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		printf("Warning: Linear texture filtering not enabled!");
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

SDL_Texture *load_texture(char *path)
{
	SDL_Texture *new_texture;

	SDL_Surface *loaded_surface = IMG_Load(path);
	if (!loaded_surface) {
		printf("Unable to load image %s! SDL Error: %s\n", path,
		       SDL_GetError());
		return NULL;
	}

	new_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
	if (!new_texture) {
		printf("Unable to create texture from %s! SDL Error: %s\n",
		       path, SDL_GetError());
	}

	SDL_FreeSurface(loaded_surface);

	return new_texture;
}

int load_media()
{
	texture = load_texture(PATH_TO_PNG);
	if (!texture) {
		printf("load_texture FAILED\n");
		return -EINVAL;
	}

	return 0;
}

// free and close stuff
void leave()
{
	SDL_DestroyTexture(texture);
	texture = NULL;

	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	//destroy window
	SDL_DestroyWindow(window);
	window = NULL;

	//quit SDL
	IMG_Quit();
	SDL_Quit();
}

static int set_and_render_viewport(SDL_Rect *vp, int x, int y, int w, int h)
{
	vp->x = x;
	vp->y = y;
	vp->w = w;
	vp->h = h;
	SDL_RenderSetViewport(renderer, vp);

	// render texture to screen
	SDL_RenderCopy(renderer, texture, NULL, NULL);

    return 0;
}

int main()
{
	int quit = 0;
	SDL_Event e;

	// top left corner viewport
	SDL_Rect top_left_vp;
	SDL_Rect top_right_vp;
	SDL_Rect bottom_vp;

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
		SDL_RenderClear(renderer);

		// render texture to screen
		set_and_render_viewport(&top_left_vp, 0, 0, SCREEN_WIDTH / 2,
					SCREEN_HEIGHT / 2);
		set_and_render_viewport(&top_right_vp, SCREEN_WIDTH / 2, 0,
					SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
		set_and_render_viewport(&bottom_vp, 0, SCREEN_HEIGHT / 2,
					SCREEN_WIDTH, SCREEN_HEIGHT / 2);

		// update screen
		SDL_RenderPresent(renderer);
	}

	leave();

	return 0;
}

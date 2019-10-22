//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PATH_TO_ARROW "../medias/arrow.png"

struct l_texture {
	SDL_Texture *texture;
	int width;
	int height;
};
// window
SDL_Window *window;
// renderer
SDL_Renderer *renderer;
// scene textures
struct l_texture arrow_texture;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// init SDL, create a window and get its surface
static int init()
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

	renderer = SDL_CreateRenderer(window, -1,
				      SDL_RENDERER_ACCELERATED |
					      SDL_RENDERER_PRESENTVSYNC);
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

static int load_ltexture_from_file(char *path, struct l_texture *in)
{
	SDL_Texture *new_texture;

	SDL_Surface *loaded_surface = IMG_Load(path);
	if (!loaded_surface) {
		printf("Unable to load image %s! SDL_image Error: %s\n", path,
		       IMG_GetError());
		return -EINVAL;
	}

	// set color key to cyan
	SDL_SetColorKey(loaded_surface, SDL_TRUE,
			SDL_MapRGB(loaded_surface->format, 0, 0xFF, 0xFF));

	// create texture from surface pixels
	new_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
	if (!new_texture) {
		printf("Unable to create texture from %s! SDL Error: %s\n",
		       path, SDL_GetError());
		return -EINVAL;
	}

	// get image dimensions
	in->width = loaded_surface->w;
	in->height = loaded_surface->h;

	// discard old surface
	SDL_FreeSurface(loaded_surface);

	in->texture = new_texture;

	return 0;
}

static void free_ltexture(struct l_texture *t)
{
	if (!t->texture)
		return;

	t->texture = NULL;
	t->width = 0;
	t->height = 0;
}

static void render(struct l_texture *in, int x, int y, SDL_Rect *clip,
		   double angle, SDL_Point *center, SDL_RendererFlip flip)
{
	if (!in)
		return;

	SDL_Rect render_quad = { x, y, in->width, in->height };

	if (clip) {
		render_quad.w = clip->w;
		render_quad.h = clip->h;
	}

	SDL_RenderCopyEx(renderer, in->texture, clip, &render_quad, angle,
			 center, flip);
}

static int load_media()
{
	int ret;

	ret = load_ltexture_from_file(PATH_TO_ARROW, &arrow_texture);
	if (ret < 0) {
		printf("Failed to load arrow texture image!\n");
		return ret;
	}

	return 0;
}

static void leave()
{
	// free loaded images
	free_ltexture(&arrow_texture);

	// destroy window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = NULL;
	window = NULL;

	//quit subsystems
	IMG_Quit();
	SDL_Quit();
}

int main()
{
	int quit = 0;
	SDL_Event e;
	double degrees = 0;
	SDL_RendererFlip flip_type = SDL_FLIP_NONE;

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
				case SDLK_q:
					degrees -= 60;
					break;

				case SDLK_s:
					degrees += 60;
					break;

				case SDLK_a:
					flip_type = SDL_FLIP_HORIZONTAL;
					break;

				case SDLK_z:
					flip_type = SDL_FLIP_NONE;
					break;

				case SDLK_e:
					flip_type = SDL_FLIP_VERTICAL;
					break;
				}
			}
		}

		// clear screen
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		// render background
		render(&arrow_texture, (SCREEN_WIDTH - arrow_texture.width) / 2,
		       (SCREEN_HEIGHT - arrow_texture.height) / 2, NULL,
		       degrees, NULL, flip_type);

		//update screen
		SDL_RenderPresent(renderer);
	}

	leave();

	return 0;
}

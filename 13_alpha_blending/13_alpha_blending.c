//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

struct l_texture {
	SDL_Texture *texture;
	int width;
	int height;
};

#define PATH_TO_FADE_FRONT "../medias/fade_front.png"
#define PATH_TO_FADE_BG "../medias/fade_bg.png"

// window
SDL_Window *window;
// renderer
SDL_Renderer *renderer;
// scene textures
struct l_texture ffront_texture;
struct l_texture fbg_texture;

const int SCREEN_WIDTH = 600;
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

static void render(struct l_texture *t, int x, int y)
{
	if (!t)
		return;

	// set rendering space and render to screen
	SDL_Rect render_quad = { x, y, t->width, t->height };

	SDL_RenderCopy(renderer, t->texture, NULL, &render_quad);
}

static int load_media()
{
	int ret;

	ret = load_ltexture_from_file(PATH_TO_FADE_FRONT, &ffront_texture);
	if (ret < 0) {
		printf("Failed to load fade front texture image!\n");
		return ret;
	}
	SDL_SetTextureBlendMode(ffront_texture.texture, SDL_BLENDMODE_BLEND);

	ret = load_ltexture_from_file(PATH_TO_FADE_BG, &fbg_texture);
	if (ret < 0) {
		printf("Failed to load fade background texture image!\n");
		return ret;
	}
	return 0;
}

static void leave()
{
	// free loaded images
	free_ltexture(&ffront_texture);
	free_ltexture(&fbg_texture);

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
	// modulation / alpha blending component
	uint8_t a = 255;

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
				//increase alpha
				case SDLK_z:
					if (a + 32 > 255)
						a = 255;
					else
						a += 32;
					break;

				//decrease alpha
				case SDLK_s:
					if (a - 32 < 0)
						a = 0;
					else
						a -= 32;
					break;
				}
			}
		}

		// clear screen
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		//render background
		render(&fbg_texture, 0, 0);

		//render blended front
		SDL_SetTextureAlphaMod(ffront_texture.texture, a);
		render(&ffront_texture, 0, 0);

		//update screen
		SDL_RenderPresent(renderer);
	}

	leave();

	return 0;
}

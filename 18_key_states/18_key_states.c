#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PATH_TO_PRESS "../medias/press.png"
#define PATH_TO_UP "../medias/up.png"
#define PATH_TO_DOWN "../medias/down.png"
#define PATH_TO_LEFT "../medias/left.png"
#define PATH_TO_RIGHT "../medias/right.png"

struct l_texture {
	SDL_Texture *texture;
	int width;
	int height;
};

const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 240;

// window
SDL_Window *window;

// renderer
SDL_Renderer *renderer;

// texture
struct l_texture texture_press;
struct l_texture texture_up;
struct l_texture texture_down;
struct l_texture texture_left;
struct l_texture texture_right;
struct l_texture *current_texture;

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

	if (TTF_Init() < 0) {
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n",
		       TTF_GetError());
		return -EINVAL;
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

static int load_media()
{
	int ret;

	ret = load_ltexture_from_file(PATH_TO_PRESS, &texture_press);
	if (ret < 0) {
		printf("Failed to load PNG!\n");
		return ret;
	}
	ret = load_ltexture_from_file(PATH_TO_UP, &texture_up);
	if (ret < 0) {
		printf("Failed to load PNG!\n");
		return ret;
	}
	ret = load_ltexture_from_file(PATH_TO_DOWN, &texture_down);
	if (ret < 0) {
		printf("Failed to load PNG!\n");
		return ret;
	}
	ret = load_ltexture_from_file(PATH_TO_LEFT, &texture_left);
	if (ret < 0) {
		printf("Failed to load PNG!\n");
		return ret;
	}
	ret = load_ltexture_from_file(PATH_TO_RIGHT, &texture_right);
	if (ret < 0) {
		printf("Failed to load PNG!\n");
		return ret;
	}

	return 0;
}

static void render(struct l_texture *t, int x, int y, SDL_Rect *clip)
{
	if (!t)
		return;

	// set rendering space and render to screen
	SDL_Rect render_quad = { x, y, t->width, t->height };

	if (clip != NULL) {
		render_quad.w = clip->w;
		render_quad.h = clip->h;
	}

	// SET 3rd PARAM to 'clip'
	SDL_RenderCopy(renderer, t->texture, clip, &render_quad);
}

static void free_ltexture(struct l_texture *t)
{
	if (!t->texture)
		return;

	t->texture = NULL;
	t->width = 0;
	t->height = 0;
}

// free and close stuff
static void leave()
{
	//Free loaded images
	free_ltexture(&texture_press);
	free_ltexture(&texture_up);
	free_ltexture(&texture_down);
	free_ltexture(&texture_left);
	free_ltexture(&texture_right);

	//Destroy window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
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

			// set texture depending key events
			const Uint8 *current_key_state =
				SDL_GetKeyboardState(NULL);
			if (current_key_state[SDL_SCANCODE_UP])
				current_texture = &texture_up;
			else if (current_key_state[SDL_SCANCODE_DOWN])
				current_texture = &texture_down;
			else if (current_key_state[SDL_SCANCODE_RIGHT])
				current_texture = &texture_right;
			else if (current_key_state[SDL_SCANCODE_LEFT])
				current_texture = &texture_left;
			else
				current_texture = &texture_press;
		}

		// clear screen
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		//render texture
		render(current_texture, 0, 0, NULL);

		//update screen
		SDL_RenderPresent(renderer);
	}

	leave();

	return 0;
}

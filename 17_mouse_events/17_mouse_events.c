#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
//#include <cmath.h>

//Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

//Button constants
#define BUTTON_WIDTH 300
#define BUTTON_HEIGHT 200
#define TOTAL_BUTTONS 4

#define PATH_TO_MOUSE_BUTTONS "../medias/mouse_buttons.png"

enum e_button_sprite {
	BUTTON_SPRITE_MOUSE_OUT = 0,
	BUTTON_SPRITE_MOUSE_OVER_MOTION = 1,
	BUTTON_SPRITE_MOUSE_DOWN = 2,
	BUTTON_SPRITE_MOUSE_UP = 3,
	BUTTON_SPRITE_TOTAL = 4
};

struct l_texture {
	SDL_Texture *texture;
	int width;
	int height;
};

// mouse button
struct l_button {
	// top left position
	SDL_Point position;

	// currrently used global sprite
	enum e_button_sprite current_sprite;
};

// window
SDL_Window *window;

// renderer
SDL_Renderer *renderer;

// used font
TTF_Font *font;

// mouse button sprites
SDL_Rect sprite_clips[TOTAL_BUTTONS];
struct l_texture button_sprite_sheet_texture;

// button objects
struct l_button buttons[TOTAL_BUTTONS];

static void l_button_set_position(struct l_button *b, int x, int y)
{
	b->position.x = x;
	b->position.y = y;
}

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

	// load faces sprites sheet
	ret = load_ltexture_from_file(PATH_TO_MOUSE_BUTTONS,
				      &button_sprite_sheet_texture);
	if (ret < 0) {
		printf("Failed to load mouse button sprite sheets image!\n");
		return ret;
	}

	// get sprites from their position on sheet
	for (int i = 0; i < TOTAL_BUTTONS; i++) {
		sprite_clips[i].x = 0;
		sprite_clips[i].y = i * 200;
		sprite_clips[i].w = BUTTON_WIDTH;
		sprite_clips[i].h = BUTTON_HEIGHT;
	}

	// set button in corners
	l_button_set_position(&buttons[0], 0, 0);
	l_button_set_position(&buttons[1], SCREEN_WIDTH - BUTTON_WIDTH, 0);
	l_button_set_position(&buttons[2], 0, SCREEN_HEIGHT - BUTTON_HEIGHT);
	l_button_set_position(&buttons[3], SCREEN_WIDTH - BUTTON_WIDTH,
			      SCREEN_HEIGHT - BUTTON_HEIGHT);

	return 0;
}

static void l_button_handle_event(struct l_button *b, SDL_Event *e)
{
	// check mouse event happened
	if (e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN ||
	    e->type == SDL_MOUSEBUTTONUP) {
		// retrieve mouse position
		int x, y;
		SDL_GetMouseState(&x, &y);

		// flag to tell wether button is in a button
		int inside = 0;

		// is mouse within button zone
		if (x >= b->position.x && x <= b->position.x + BUTTON_WIDTH &&
		    y >= b->position.y && y <= b->position.y + BUTTON_HEIGHT)
			inside = 1;

		if (!inside) {
			b->current_sprite = BUTTON_SPRITE_MOUSE_OUT;
		} else {
			switch (e->type) {
			case SDL_MOUSEMOTION:
				b->current_sprite =
					BUTTON_SPRITE_MOUSE_OVER_MOTION;
				break;
			case SDL_MOUSEBUTTONDOWN:
				b->current_sprite = BUTTON_SPRITE_MOUSE_DOWN;
				break;
			case SDL_MOUSEBUTTONUP:
				b->current_sprite = BUTTON_SPRITE_MOUSE_UP;
				break;
			}
		}
	}
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

static void l_button_render(struct l_button *b)
{
	// show current button sprite
	render(&button_sprite_sheet_texture, b->position.x, b->position.y,
	       &sprite_clips[b->current_sprite]);
}

static void free_ltexture(struct l_texture *t)
{
	if (!t->texture)
		return;

	t->texture = NULL;
	t->width = 0;
	t->height = 0;
}

static void leave()
{
	//Free loaded images
	free_ltexture(&button_sprite_sheet_texture);

	//Free global font
	TTF_CloseFont(font);
	font = NULL;

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

			// handle button events
			for (int i = 0; i < TOTAL_BUTTONS; i++)
				l_button_handle_event(&buttons[i], &e);
		}
		// clear screen
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		// render buttons
		for (int i = 0; i < TOTAL_BUTTONS; i++)
			l_button_render(&buttons[i]);

		// update screen
		SDL_RenderPresent(renderer);

		usleep(50000);
	}
	leave();

	return 0;
}

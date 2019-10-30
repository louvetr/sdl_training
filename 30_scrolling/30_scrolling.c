//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PATH_TO_LION "../medias/lion_head.png"
#define PATH_TO_BG "../medias/scrolling_bg.png"
#define LEVEL_WIDTH 1280
#define LEVEL_HEIGHT 960
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MOVING_OBJECT_WIDTH 64
#define MOVING_OBJECT_HEIGHT 64
#define MOVING_OBJECT_MAX_VELOCITY 10

struct l_texture {
	SDL_Texture *texture;
	int width;
	int height;
};

struct l_moving_object {
	int pos_x;
	int pos_y;
	int vel_x;
	int vel_y;
};

// window
SDL_Window *window;
// renderer
SDL_Renderer *renderer;
// scene textures
struct l_texture lion_head_texture;
struct l_texture stage_texture;

///////////////////////////////////////////////////////
// moving object functions
///////////////////////////////////////////////////////

static void mo_handle_event(struct l_moving_object *mo, SDL_Event e)
{
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_UP:
			mo->vel_y -= MOVING_OBJECT_MAX_VELOCITY;
			break;
		case SDLK_DOWN:
			mo->vel_y += MOVING_OBJECT_MAX_VELOCITY;
			break;
		case SDLK_LEFT:
			mo->vel_x -= MOVING_OBJECT_MAX_VELOCITY;
			break;
		case SDLK_RIGHT:
			mo->vel_x += MOVING_OBJECT_MAX_VELOCITY;
			break;
		}
	} else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_UP:
			mo->vel_y += MOVING_OBJECT_MAX_VELOCITY;
			break;
		case SDLK_DOWN:
			mo->vel_y -= MOVING_OBJECT_MAX_VELOCITY;
			break;
		case SDLK_LEFT:
			mo->vel_x += MOVING_OBJECT_MAX_VELOCITY;
			break;
		case SDLK_RIGHT:
			mo->vel_x -= MOVING_OBJECT_MAX_VELOCITY;
			break;
		}
	}
}

static void mo_move(struct l_moving_object *mo)
{
	// move horizontally
	mo->pos_x += mo->vel_x;
	// do not go outside screen
	if (mo->pos_x < 0 || mo->pos_x > LEVEL_WIDTH - MOVING_OBJECT_WIDTH)
		mo->pos_x -= mo->vel_x;

	// move vertically
	mo->pos_y += mo->vel_y;
	// do not go outside screen
	if (mo->pos_y < 0 || mo->pos_y > LEVEL_HEIGHT - MOVING_OBJECT_HEIGHT)
		mo->pos_y -= mo->vel_y;

	printf("mo(x,y) = (%d, %d)\n", mo->pos_x, mo->pos_y);
}

///////////////////////////////////////////////////////
// common functions
///////////////////////////////////////////////////////

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

static void render(struct l_texture *t, int x, int y, SDL_Rect *clip)
{
	// Set rendering space and render to screen
	SDL_Rect render_quad = { x, y, t->width, t->height };

	//Set clip rendering dimensions
	if (clip != NULL) {
		render_quad.w = clip->w;
		render_quad.h = clip->h;
	}

	SDL_RenderCopyEx(renderer, t->texture, clip, &render_quad, 0, NULL, 0);
}

static int load_media()
{
	int ret;

	// load character
	ret = load_ltexture_from_file(PATH_TO_LION, &lion_head_texture);
	if (ret < 0) {
		printf("Failed to load foo texture image!\n");
		return ret;
	}

	// load background
	ret = load_ltexture_from_file(PATH_TO_BG, &stage_texture);
	if (ret < 0) {
		printf("Failed to load foo texture image!\n");
		return ret;
	}
	return 0;
}

static void leave()
{
	// free loaded images
	free_ltexture(&lion_head_texture);
	free_ltexture(&stage_texture);

	// destroy window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = NULL;
	window = NULL;

	//quit subsystems
	IMG_Quit();
	SDL_Quit();
}

///////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////

int main()
{
	int quit = 0;
	SDL_Event e;
	struct l_moving_object mo = {
		.pos_x = LEVEL_WIDTH / 2 - MOVING_OBJECT_WIDTH,
		.pos_y = LEVEL_HEIGHT / 2 - MOVING_OBJECT_HEIGHT,
		.vel_x = 0,
		.vel_y = 0,
	};
	SDL_Rect camera = {
		.x = 0, .y = 0, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT,
	};

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

			mo_handle_event(&mo, e);
		}

		// move the moving object
		mo_move(&mo);

		// center camera on the moving object
		camera.x =
			mo.pos_x + MOVING_OBJECT_WIDTH / 2 - SCREEN_WIDTH / 2;
		camera.y =
			mo.pos_y + MOVING_OBJECT_HEIGHT / 2 - SCREEN_HEIGHT / 2;

		// keep the camera in bounds
		if (camera.x < 0)
			camera.x = 0;
		if (camera.y < 0)
			camera.y = 0;
		if (camera.x > LEVEL_WIDTH - camera.w)
			camera.x = LEVEL_WIDTH - camera.w;
		if (camera.y > LEVEL_HEIGHT - camera.h)
			camera.y = LEVEL_HEIGHT - camera.h;

		printf("camera.x,y = %d, %d\n", camera.x, camera.y);

		// clear screen
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		//render backgrountd
		render(&stage_texture, 0, 0, &camera);

		//render character
		if (camera.x + MOVING_OBJECT_WIDTH > SCREEN_WIDTH)
			camera.x = SCREEN_WIDTH - MOVING_OBJECT_WIDTH;
		if (camera.y + MOVING_OBJECT_HEIGHT > SCREEN_HEIGHT)
			camera.y = SCREEN_HEIGHT - MOVING_OBJECT_HEIGHT;
		render(&lion_head_texture, camera.x, camera.y, NULL);

		//update screen
		SDL_RenderPresent(renderer);

		SDL_Delay(1000 / 60);
	}

	leave();

	return 0;
}

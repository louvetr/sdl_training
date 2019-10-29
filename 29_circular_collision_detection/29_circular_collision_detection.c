//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

///////////////////////////////////////////////////////
// defines
///////////////////////////////////////////////////////

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MOVING_OBJECT_WIDTH 64
#define MOVING_OBJECT_HEIGHT 64
#define MOVING_OBJECT_MAX_VELOCITY 10
#define PATH_TO_LION "../medias/lion_head.png"

///////////////////////////////////////////////////////
// structures
///////////////////////////////////////////////////////

struct l_circle {
	int x;
	int y;
	int r;
};

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
	SDL_Rect hit_box_rect;
	struct l_circle hit_box_circle;
};

///////////////////////////////////////////////////////
// global variables
///////////////////////////////////////////////////////

// window
SDL_Window *window;
// renderer
SDL_Renderer *renderer;
// scene textures
struct l_texture lion_head_texture;

///////////////////////////////////////////////////////
// collision detection
///////////////////////////////////////////////////////

static int distance_squared(int x1, int y1, int x2, int y2)
{
	int delta_x = x2 - x1;
	int delta_y = y2 - y1;

	return delta_x * delta_x + delta_y * delta_y;
}

static int check_collision_rect_rect(SDL_Rect a, SDL_Rect b)
{
	// sides of the rectangles
	int up_a, down_a, left_a, right_a;
	int up_b, down_b, left_b, right_b;

	// sides of rect A
	left_a = a.x;
	right_a = a.x + a.w;
	up_a = a.y;
	down_a = a.y + a.h;

	// sides of rect A
	left_b = b.x;
	right_b = b.x + b.w;
	up_b = b.y;
	down_b = b.y + b.h;

	if (left_a >= right_b || right_a <= left_b || up_a >= down_b ||
	    down_a <= up_b)
		return 0;
	else
		return 1;
}

static int check_collision_circle_circle(struct l_circle a, struct l_circle b)
{
	int total_radius_squared = (a.r + b.r) * (a.r + b.r);

	// is centers distance is smaller the sum of their radius
	if (distance_squared(a.x, a.y, b.x, b.y) < total_radius_squared)
		return 1;
	else
		return 0;
}

static int check_collision_rect_circle(SDL_Rect b, struct l_circle a)
{
	// closest point of collision box
	int ret, cx, cy;

	// find closest x offset
	if (a.x < b.x)
		cx = b.x;
	else if (a.x > b.x + b.w)
		cx = b.x + b.w;
	else
		cx = a.x;

	// find closest y offset
	if (a.y < b.y)
		cy = b.y;
	else if (a.y > b.y + b.h)
		cy = b.y + b.h;
	else
		cy = a.y;

	// check if closest point is inside the circle
	if (distance_squared(a.x, a.y, cx, cy) < a.r * a.r)
		ret = 1;
	else
		ret = 0;

	return ret;
}

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

static void mo_move(struct l_moving_object *mo, SDL_Rect wall,
		    struct l_circle circle)
{
	// move horizontally
	mo->pos_x += mo->vel_x;
	mo->hit_box_rect.x = mo->pos_x;

	// do not go outside screen
	if (mo->pos_x < 0 || mo->pos_x > SCREEN_WIDTH ||
	    check_collision_rect_rect(mo->hit_box_rect, wall) ||
	    check_collision_rect_circle(mo->hit_box_rect, circle)) {
		mo->pos_x -= mo->vel_x;
		mo->hit_box_rect.x = mo->pos_x;
	}

	// move vertically
	mo->pos_y += mo->vel_y;
	mo->hit_box_rect.y = mo->pos_y;

	// do not go outside screen
	if (mo->pos_y < 0 || mo->pos_y > SCREEN_HEIGHT ||
	    check_collision_rect_rect(mo->hit_box_rect, wall) ||
	    check_collision_rect_circle(mo->hit_box_rect, circle)) {
		mo->pos_y -= mo->vel_y;
		mo->hit_box_rect.y = mo->pos_y;
	}
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

static void render(struct l_texture *t, int x, int y)
{
	// Set rendering space and render to screen
	SDL_Rect render_quad = { x, y, t->width, t->height };
	SDL_RenderCopy(renderer, t->texture, NULL, &render_quad);
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

	return 0;
}

static void leave()
{
	// free loaded images
	free_ltexture(&lion_head_texture);

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
		.pos_x = 0,
		.pos_y = 0,
		.vel_x = 0,
		.vel_y = 0,
		.hit_box_rect.w = MOVING_OBJECT_WIDTH,
		.hit_box_rect.h = MOVING_OBJECT_HEIGHT,
		.hit_box_circle.r = MOVING_OBJECT_WIDTH / 2,
	};

	struct l_circle circle = {
		.x = 100, .y = 200, .r = 32,
	};

	SDL_Rect wall = {
		.x = 300, .y = 80, .w = 40, .h = 300,
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

		mo_move(&mo, wall, circle);

		// clear screen
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		//Render wall
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawRect(renderer, &wall);

		//render character
		render(&lion_head_texture, mo.pos_x, mo.pos_y);

		// render circle / other lion head obstacle
		render(&lion_head_texture, circle.x, circle.y);

		//update screen
		SDL_RenderPresent(renderer);

		SDL_Delay(1000 / 60);
	}

	leave();

	return 0;
}

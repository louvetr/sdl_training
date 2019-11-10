//Using SDL, SDL_image, standard IO, strings, and file streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MO_WIDTH 64
#define MO_HEIGHT 64
#define MO_MAX_VELOCITY 10

//Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

//The dimensions of the level
#define LEVEL_WIDTH 1280
#define LEVEL_HEIGHT 960

//Tile constants
#define TILE_WIDTH 80
#define TILE_HEIGHT 80
#define TOTAL_TILES 192
#define TOTAL_TILE_SPRITES 12

//The different tile sprites
#define TILE_WOOD 0
#define TILE_MARBLE 1
#define TILE_SKY 2
#define TILE_CENTER 3
#define TILE_TOP 4
#define TILE_TOPRIGHT 5
#define TILE_RIGHT 6
#define TILE_BOTTOMRIGHT 7
#define TILE_BOTTOM 8
#define TILE_BOTTOMLEFT 9
#define TILE_LEFT 10
#define TILE_TOPLEFT 11

#define PATH_TO_LION "../medias/lion_head.png"
#define PATH_TO_TILES "../medias/tiles_array.png"
#define PATH_TO_MAP "../medias/39.map"

struct l_tile {
	// attribute of the tile
	SDL_Rect box;
	// tile type
	int type;
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
	SDL_Rect box;
};

SDL_Rect g_tile_clips[TOTAL_TILE_SPRITES] = { 0 };

///////////////////////////////////////////////////////
// global variables
///////////////////////////////////////////////////////

// window
SDL_Window *g_window;
// renderer
SDL_Renderer *g_renderer;
// scene textures
struct l_texture g_mo_texture;
struct l_texture g_tiles_texture;

///////////////////////////////////////////////////////
// static functions declarations
///////////////////////////////////////////////////////

static void tile_clips_init();
static void tile_init(struct l_tile *tile, int x, int y, int tile_type);

///////////////////////////////////////////////////////
// common functions
///////////////////////////////////////////////////////

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
	new_texture = SDL_CreateTextureFromSurface(g_renderer, loaded_surface);
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

// init SDL, create a g_window and get its surface
static int init()
{
	int ret;

	tile_clips_init();

	// init SDL
	ret = SDL_Init(SDL_INIT_VIDEO);
	if (ret < 0) {
		printf("SDL_Init ERROR: %s\n", SDL_GetError());
		return ret;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		printf("Warning: Linear texture filtering not enabled!");
	}

	// create g_window
	g_window = SDL_CreateWindow("SDL_tuto", SDL_WINDOWPOS_UNDEFINED,
				    SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
				    SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!g_window) {
		printf("SDL_SetVideoMode ERROR: %s\n", SDL_GetError());
		return -1;
	}

	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
	if (!g_renderer) {
		printf("Renderer could not be created! SDL Error: %s\n",
		       SDL_GetError());
		return -1;
	}

	SDL_SetRenderDrawColor(g_renderer, 0x00, 0xFF, 0xFF, 0x00);

	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
		       IMG_GetError());
		return -1;
	}

	return 0;
}

static int set_tiles(struct l_tile *tiles, int nb_tiles)
{
	int ret = 0;
	uint8_t tile_type;

	// tiles offsets
	int x = 0, y = 0;

	// open the map
	FILE *f = fopen(PATH_TO_MAP, "r");
	if (f == NULL) {
		printf("Failed to fopen map file\n");
		return -EINVAL;
	}

	// TODO (if not TUTO): check file size match with TOTAL_TILES
	for (int i = 0; i < TOTAL_TILES; i++) {
		ret = fread(&tile_type, 1, 1, f);
		if (ret < 1) {
			printf("Failed to fread map file at byte #%d\n", i);
			return -EINVAL;
		}
		// skip new line
		if (tile_type == '\n') {
			i--;
			continue;
		}
		// ascii offset shift to start to 0
		tile_type -= 48;

		if (tile_type >= TOTAL_TILE_SPRITES) {
			printf("invalid tile_type at byte #%d = %d\n", i,
			       tile_type + 48);
			return -EINVAL;
		}
		tile_init(&tiles[i], x, y, tile_type);
		// move to next tile
		x += TILE_WIDTH;
		// change line if need be
		if (x >= LEVEL_WIDTH) {
			x = 0;
			y += TILE_HEIGHT;
		}
	}

	fclose(f);
	return 0;
}

static void texture_render(struct l_texture *t, int x, int y, SDL_Rect *clip)
{
	// Set rendering space and render to screen
	SDL_Rect render_quad = { x, y, t->width, t->height };

	// Set clip rendering dimensions
	if (clip != NULL) {
		render_quad.w = clip->w;
		render_quad.h = clip->h;
	}

	SDL_RenderCopyEx(g_renderer, t->texture, clip, &render_quad, 0, NULL,
			 0);
}

static int load_media(struct l_tile *tiles, int nb_tiles)
{
	int ret;

	// load character
	ret = load_ltexture_from_file(PATH_TO_LION, &g_mo_texture);
	if (ret < 0) {
		printf("Failed to load lion texture image!\n");
		return ret;
	}
	// load tile sheet
	ret = load_ltexture_from_file(PATH_TO_TILES, &g_tiles_texture);
	if (ret < 0) {
		printf("Failed to load tiles texture image!\n");
		return ret;
	}
	// load level/tile map file
	ret = set_tiles(tiles, nb_tiles);
	if (ret < 0) {
		printf("Failed to load map file!\n");
		return ret;
	}

	return 0;
}

static void leave()
{
	// free loaded images
	free_ltexture(&g_mo_texture);
	//free_ltexture(&background_texture);

	// destroy g_window
	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	g_renderer = NULL;
	g_window = NULL;

	//quit subsystems
	IMG_Quit();
	SDL_Quit();
}

static void tile_clips_init()
{
	g_tile_clips[TILE_WOOD].x = 0;
	g_tile_clips[TILE_WOOD].y = 0;
	g_tile_clips[TILE_WOOD].w = TILE_WIDTH;
	g_tile_clips[TILE_WOOD].h = TILE_HEIGHT;

	g_tile_clips[TILE_MARBLE].x = 0;
	g_tile_clips[TILE_MARBLE].y = 80;
	g_tile_clips[TILE_MARBLE].w = TILE_WIDTH;
	g_tile_clips[TILE_MARBLE].h = TILE_HEIGHT;

	g_tile_clips[TILE_SKY].x = 0;
	g_tile_clips[TILE_SKY].y = 160;
	g_tile_clips[TILE_SKY].w = TILE_WIDTH;
	g_tile_clips[TILE_SKY].h = TILE_HEIGHT;

	g_tile_clips[TILE_TOPLEFT].x = 80;
	g_tile_clips[TILE_TOPLEFT].y = 0;
	g_tile_clips[TILE_TOPLEFT].w = TILE_WIDTH;
	g_tile_clips[TILE_TOPLEFT].h = TILE_HEIGHT;

	g_tile_clips[TILE_LEFT].x = 80;
	g_tile_clips[TILE_LEFT].y = 80;
	g_tile_clips[TILE_LEFT].w = TILE_WIDTH;
	g_tile_clips[TILE_LEFT].h = TILE_HEIGHT;

	g_tile_clips[TILE_BOTTOMLEFT].x = 80;
	g_tile_clips[TILE_BOTTOMLEFT].y = 160;
	g_tile_clips[TILE_BOTTOMLEFT].w = TILE_WIDTH;
	g_tile_clips[TILE_BOTTOMLEFT].h = TILE_HEIGHT;

	g_tile_clips[TILE_TOP].x = 160;
	g_tile_clips[TILE_TOP].y = 0;
	g_tile_clips[TILE_TOP].w = TILE_WIDTH;
	g_tile_clips[TILE_TOP].h = TILE_HEIGHT;

	g_tile_clips[TILE_CENTER].x = 160;
	g_tile_clips[TILE_CENTER].y = 80;
	g_tile_clips[TILE_CENTER].w = TILE_WIDTH;
	g_tile_clips[TILE_CENTER].h = TILE_HEIGHT;

	g_tile_clips[TILE_BOTTOM].x = 160;
	g_tile_clips[TILE_BOTTOM].y = 160;
	g_tile_clips[TILE_BOTTOM].w = TILE_WIDTH;
	g_tile_clips[TILE_BOTTOM].h = TILE_HEIGHT;

	g_tile_clips[TILE_TOPRIGHT].x = 240;
	g_tile_clips[TILE_TOPRIGHT].y = 0;
	g_tile_clips[TILE_TOPRIGHT].w = TILE_WIDTH;
	g_tile_clips[TILE_TOPRIGHT].h = TILE_HEIGHT;

	g_tile_clips[TILE_RIGHT].x = 240;
	g_tile_clips[TILE_RIGHT].y = 80;
	g_tile_clips[TILE_RIGHT].w = TILE_WIDTH;
	g_tile_clips[TILE_RIGHT].h = TILE_HEIGHT;

	g_tile_clips[TILE_BOTTOMRIGHT].x = 240;
	g_tile_clips[TILE_BOTTOMRIGHT].y = 160;
	g_tile_clips[TILE_BOTTOMRIGHT].w = TILE_WIDTH;
	g_tile_clips[TILE_BOTTOMRIGHT].h = TILE_HEIGHT;
}

///////////////////////////////////////////////////////
// collision detection
///////////////////////////////////////////////////////

static int check_collision(SDL_Rect a, SDL_Rect b)
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

static int check_wall_touched(SDL_Rect *box, struct l_tile *tiles, int nb_tiles)
{
	// go through tiles
	for (int i = 0; i < nb_tiles; i++) {
		// check tile is of wall type
		if (tiles[i].type >= TILE_CENTER &&
		    tiles[i].type <= TILE_TOPLEFT) {
			// collision with the wall ?
			if (check_collision(*box, tiles[i].box))
				return 1;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////
// moving object functions
///////////////////////////////////////////////////////

static void mo_handle_event(struct l_moving_object *mo, SDL_Event e)
{
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_UP:
			mo->vel_y -= MO_MAX_VELOCITY;
			break;
		case SDLK_DOWN:
			mo->vel_y += MO_MAX_VELOCITY;
			break;
		case SDLK_LEFT:
			mo->vel_x -= MO_MAX_VELOCITY;
			break;
		case SDLK_RIGHT:
			mo->vel_x += MO_MAX_VELOCITY;
			break;
		}
	} else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_UP:
			mo->vel_y += MO_MAX_VELOCITY;
			break;
		case SDLK_DOWN:
			mo->vel_y -= MO_MAX_VELOCITY;
			break;
		case SDLK_LEFT:
			mo->vel_x += MO_MAX_VELOCITY;
			break;
		case SDLK_RIGHT:
			mo->vel_x -= MO_MAX_VELOCITY;
			break;
		}
	}
}

static void mo_move(struct l_moving_object *mo, struct l_tile *tiles,
		    int nb_tiles)
{
	// move horizontally
	mo->pos_x += mo->vel_x;
	mo->box.x = mo->pos_x;

	// do not go outside screen
	if (mo->pos_x < 0 || mo->pos_x + MO_WIDTH > LEVEL_WIDTH ||
	    check_wall_touched(&mo->box, tiles, nb_tiles)) {
		mo->pos_x -= mo->vel_x;
		mo->box.x = mo->pos_x;
	}

	// move vertically
	mo->pos_y += mo->vel_y;
	mo->box.y = mo->pos_y;

	// do not go outside screen
	if (mo->pos_y < 0 || mo->pos_y + MO_HEIGHT > LEVEL_HEIGHT ||
	    check_wall_touched(&mo->box, tiles, nb_tiles)) {
		mo->pos_y -= mo->vel_y;
		mo->box.y = mo->pos_y;
	}
}

static void mo_set_camera(struct l_moving_object *mo, SDL_Rect *camera)
{
	//Center the camera over the dot
	camera->x = (mo->box.x + MO_WIDTH / 2) - SCREEN_WIDTH / 2;
	camera->y = (mo->box.y + MO_HEIGHT / 2) - SCREEN_HEIGHT / 2;

	//Keep the camera in bounds
	if (camera->x < 0)
		camera->x = 0;
	if (camera->y < 0)
		camera->y = 0;
	if (camera->x > LEVEL_WIDTH - camera->w)
		camera->x = LEVEL_WIDTH - camera->w;
	if (camera->y > LEVEL_HEIGHT - camera->h)
		camera->y = LEVEL_HEIGHT - camera->h;
}

void mo_render(struct l_moving_object *mo, SDL_Rect *camera)
{
	//Show the dot
	texture_render(&g_mo_texture, mo->box.x - camera->x,
		       mo->box.y - camera->y, NULL);
}

///////////////////////////////////////////////////////
// tile functions
///////////////////////////////////////////////////////

static void tile_init(struct l_tile *tile, int x, int y, int tile_type)
{
	// set collision box
	tile->box.x = x;
	tile->box.y = y;
	tile->box.w = TILE_WIDTH;
	tile->box.h = TILE_HEIGHT;

	// set type
	tile->type = tile_type;
}

static void tile_render(struct l_tile *tile, SDL_Rect *camera)
{
	// show the tile only if it is on the screen
	if (check_collision(*camera, tile->box))
		texture_render(&g_tiles_texture, tile->box.x - camera->x,
			       tile->box.y - camera->y,
			       &g_tile_clips[tile->type]);
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
		.box.w = MO_WIDTH,
		.box.h = MO_HEIGHT,
	};
	SDL_Rect camera = {
		.x = 0, .y = 0, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT,
	};

	struct l_tile *tileset = calloc(TOTAL_TILES, sizeof(struct l_tile));
	if (tileset == NULL) {
		printf("Failed to alloc tileset!\n");
		return -EINVAL;
	}

	init();
	load_media(tileset, TOTAL_TILES);

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

		mo_move(&mo, tileset, TOTAL_TILES);
		mo_set_camera(&mo, &camera);

		// clear screen
		SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(g_renderer);

		// render level
		for (int i = 0; i < TOTAL_TILES; i++)
			tile_render(&tileset[i], &camera);

		//render character
		mo_render(&mo, &camera);

		//update screen
		SDL_RenderPresent(g_renderer);

		SDL_Delay(1000 / 60);
	}

	leave();
	free(tileset);

	return 0;
}
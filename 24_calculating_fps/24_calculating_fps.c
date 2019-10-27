//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

struct l_texture {
	SDL_Texture *texture;
	int width;
	int height;
};

struct l_timer {
	Uint32 start_ticks;
	Uint32 paused_ticks;
	int paused;
	int started;
};

// window
SDL_Window *window;
// renderer
SDL_Renderer *renderer;
// used font
TTF_Font *font;
// text textures
struct l_texture time_texture;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

static void timer_start(struct l_timer *t)
{
	t->started = 1;
	t->paused = 0;
	t->start_ticks = SDL_GetTicks();
	t->paused_ticks = 0;
}

static Uint32 timer_get_ticks(struct l_timer *t)
{
	if (!t->started)
		return 0;

	if (t->paused)
		return t->paused_ticks;
	else
		return SDL_GetTicks() - t->start_ticks;
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

static int load_from_rendered_text(struct l_texture *t, char *string,
				   SDL_Color text_color)
{
	SDL_Surface *text_surface =
		TTF_RenderText_Solid(font, string, text_color);

	if (!text_surface) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n",
		       TTF_GetError());
		return -EINVAL;
	}

	t->texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	if (!t->texture) {
		printf("Unable to create texture from rendered text! SDL Error: %s\n",
		       SDL_GetError());
		return -EINVAL;
	}

	t->width = text_surface->w;
	t->height = text_surface->h;

	SDL_FreeSurface(text_surface);
	return 0;
}

static int load_media()
{
	int ret = 0;

	//Open the font
	font = TTF_OpenFont("../medias/QuaeriteRegnumDei.otf", 42);
	if (!font) {
		printf("Failed to load lazy font! SDL_ttf Error: %s\n",
		       TTF_GetError());
		return -EINVAL;
	}

	return ret;
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

static void leave()
{
	//Free loaded images
	free_ltexture(&time_texture);

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
	int quit = 0, ret;
	SDL_Event e;
	//Set text color as black
	SDL_Color text_color = { 0, 0, 0, 255 };
	// current start time
	char time_str[64] = { 0 };
	// appli timer
	struct l_timer fps_timer = { 0 };
	int counted_frames = 0;

	init();
	load_media();

	timer_start(&fps_timer);

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

		// calculate average fps and correct it
		float avg_fps = (float)counted_frames /
				(timer_get_ticks(&fps_timer) / 1000.f);
		if (avg_fps > 1000000)
			avg_fps = 0;

		// set text to render
		snprintf(time_str, 64, "Average FPS = %d", (int)avg_fps);

		// render text
		ret = load_from_rendered_text(&time_texture, time_str,
					      text_color);
		if (ret < 0)
			printf("Failed to render text texture!\n");

		// clear screen
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		// render textures
		render(&time_texture, (SCREEN_WIDTH - time_texture.width) / 2,
		       (SCREEN_HEIGHT - time_texture.height) / 2, NULL);

		//update screen
		SDL_RenderPresent(renderer);
		counted_frames++;
	}

	leave();

	return 0;
}
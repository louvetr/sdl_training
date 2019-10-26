#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PATH_TO_PNG "../medias/lesson21.png"

struct l_texture {
	SDL_Texture *texture;
	int width;
	int height;
};

const int SCREEN_WIDTH = 648;
const int SCREEN_HEIGHT = 480;

// window
SDL_Window *window;
// renderer
SDL_Renderer *renderer;
// texture
struct l_texture texture;

// music to be played
Mix_Music *music;
// sounc_effects
Mix_Chunk *sfx_tires;
Mix_Chunk *sfx_door;
Mix_Chunk *sfx_handbrake;
Mix_Chunk *sfx_horn;

// init SDL, create a window and get its surface
static int init()
{
	int ret;

	// init SDL
	ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
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

	// init PNG loading
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
		       IMG_GetError());
		return -1;
	}

	// init Fonts management
	if (TTF_Init() < 0) {
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n",
		       TTF_GetError());
		return -EINVAL;
	}

	// init Music and SFX management
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_ttf could not initialize! SDL_Mixer Error: %s\n",
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

	// load png
	ret = load_ltexture_from_file(PATH_TO_PNG, &texture);
	if (ret < 0) {
		printf("Failed to load PNG!\n");
		return ret;
	}

	// load music
	music = Mix_LoadMUS("../medias/F1_race_96kbps.mp3");
	if (!music) {
		printf("Failed to music\n");
		return -EINVAL;
	}

	//Load sound effects
	sfx_tires = Mix_LoadWAV("../medias/criss_court_1.wav");
	if (!sfx_tires) {
		printf("Failed to load sfx_tires! SDL_mixer Error: %s\n",
		       Mix_GetError());
		return -EINVAL;
	}
	sfx_door = Mix_LoadWAV("../medias/fermeture_porte.wav");
	if (!sfx_door) {
		printf("Failed to load sfx_door! SDL_mixer Error: %s\n",
		       Mix_GetError());
		return -EINVAL;
	}
	sfx_handbrake = Mix_LoadWAV("../medias/frein_a_main.wav");
	if (!sfx_handbrake) {
		printf("Failed to load sfx_tires! sfx_handbrake Error: %s\n",
		       Mix_GetError());
		return -EINVAL;
	}
	sfx_horn = Mix_LoadWAV("../medias/klaxon.wav");
	if (!sfx_horn) {
		printf("Failed to load sfx_horn! SDL_mixer Error: %s\n",
		       Mix_GetError());
		return -EINVAL;
	}

    printf("media loaded\n");

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
	free_ltexture(&texture);

	// free music
	Mix_FreeMusic(music);
	music = NULL;

	//Free the sound effects
	Mix_FreeChunk(sfx_handbrake);
	Mix_FreeChunk(sfx_horn);
	Mix_FreeChunk(sfx_tires);
	Mix_FreeChunk(sfx_door);
	sfx_handbrake = NULL;
	sfx_horn = NULL;
	sfx_tires = NULL;
	sfx_door = NULL;

	//Destroy window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	//Quit SDL subsystems
	Mix_Quit();
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

			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_a:
					Mix_PlayChannel(-1, sfx_door, 0);
					break;
				case SDLK_z:
					Mix_PlayChannel(-1, sfx_tires, 0);
					break;
				case SDLK_e:
					Mix_PlayChannel(-1, sfx_handbrake, 0);
					break;
				case SDLK_r:
					Mix_PlayChannel(-1, sfx_horn, 0);
					break;
				case SDLK_p:
					if (!Mix_PlayingMusic()) {
						// play music if none is playing
						Mix_PlayMusic(music, -1);
					} else {
						// is music paused
						if (Mix_PausedMusic()) {
                            // play music
                            Mix_ResumeMusic();
						} else {
                            // pause music
                            Mix_PauseMusic();
						}
					}
					break;
				case SDLK_s:
                    // stop music
					Mix_HaltMusic();
					break;
				}
			}
		}

		// clear screen
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		//render texture
		render(&texture, 0, 0, NULL);

		//update screen
		SDL_RenderPresent(renderer);
	}

	leave();

	return 0;
}

/* loadseq: does what Starfighter does at start-up, one step per word of a
 * list, so the step that brings the emulator down can be found without
 * the game: each step is noted in PROGDIR:loadseq.txt first (opened and
 * closed per line, so it is on the disk after the emulator goes).
 *
 *   LoadSeq WORD.. | @LISTFILE      (a list file: words, any white space)
 *
 *   video          SDL_Init(VIDEO|AUDIO|JOYSTICK|GAMECONTROLLER), a 640x480
 *                  resizable window, the renderer, its logical size and a
 *                  640x480 ARGB8888 streaming texture (as the game)
 *   audio          Mix_OpenAudio(44100, S16, 2, 1024)
 *   NAME.png|.jpg  IMG_Load, SDL_ConvertSurface to the game's 32-bit
 *                  surface format, SDL_SetColorKey black; kept
 *   raw:NAME       IMG_Load only; kept
 *   conv:NAME      IMG_Load and SDL_ConvertSurface, no colour key; kept
 *   key:NAME       IMG_Load and SDL_SetColorKey, not converted; kept
 *   surface        only the game's 640x480 32-bit surface (no SDL_Init,
 *                  no window): what NAME.png converts to without video
 *   NAME.ogg       Mix_LoadWAV; kept
 *   music:NAME     Mix_LoadMUS and Mix_PlayMusic
 *   frame          one SDL_UpdateTexture/RenderCopy/RenderPresent
 *   wait:N         SDL_Delay(N ms)
 *   free           frees everything kept
 *
 * m68k-amigaos-gcc -noixemul -m68040 -m68881 -O2 loadseq.c -o LoadSeq \
 *     -I$SDL2KIT/include/SDL2 -L$SDL2KIT/lib -lSDL2_image -lSDL2_mixer -lSDL2 -lm
 * The AmiCradle Team. GPL 3 or later, like Starfighter. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

unsigned long __stack = 64 * 1024;

static void note(const char *fmt, ...)
{
	static int line;
	va_list ap;
	FILE *fp = fopen("PROGDIR:loadseq.txt", line ? "a" : "w");
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	putchar('\n');
	fflush(stdout);
	if (fp) {
		fprintf(fp, "%04d ", line);
		va_start(ap, fmt);
		vfprintf(fp, fmt, ap);
		va_end(ap);
		fputc('\n', fp);
		fclose(fp);
	}
	line++;
}

#define MAXKEPT 512
static SDL_Surface *surfaces[MAXKEPT];
static Mix_Chunk *chunks[MAXKEPT];
static int nsurf, nchunk;
static SDL_Surface *screen;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static Mix_Music *music;

static void step(const char *w)
{
	size_t n = strlen(w);
	const char *ext = n > 4 ? w + n - 4 : "";

	note("%s", w);
	if (strcmp(w, "video") == 0) {
		SDL_RendererInfo ri;
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
			note("  SDL_Init: %s", SDL_GetError());
		screen = SDL_CreateRGBSurface(0, 640, 480, 32, 0xff0000, 0xff00, 0xff, 0xff000000);
		window = SDL_CreateWindow("loadseq", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			640, 480, SDL_WINDOW_RESIZABLE);
		note("  window %p", (void *)window);
		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer && SDL_GetRendererInfo(renderer, &ri) == 0)
			note("  renderer %s", ri.name);
		SDL_RenderSetLogicalSize(renderer, 640, 480);
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, 640, 480);
		note("  texture %p", (void *)texture);
	} else if (strcmp(w, "surface") == 0) {
		screen = SDL_CreateRGBSurface(0, 640, 480, 32, 0xff0000, 0xff00, 0xff, 0xff000000);
		note("  surface %p", (void *)screen);
	} else if (strcmp(w, "audio") == 0) {
		if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0)
			note("  Mix_OpenAudio: %s", Mix_GetError());
	} else if (strcmp(w, "frame") == 0) {
		SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	} else if (strncmp(w, "wait:", 5) == 0) {
		SDL_Delay(atoi(w + 5));
	} else if (strcmp(w, "free") == 0) {
		while (nsurf)
			SDL_FreeSurface(surfaces[--nsurf]);
		while (nchunk)
			Mix_FreeChunk(chunks[--nchunk]);
	} else if (strncmp(w, "music:", 6) == 0) {
		music = Mix_LoadMUS(w + 6);
		note("  music %p %s", (void *)music, music ? "" : Mix_GetError());
		if (music)
			Mix_PlayMusic(music, -1);
	} else if (strcmp(ext, ".ogg") == 0 || strcmp(ext, ".wav") == 0) {
		Mix_Chunk *c = Mix_LoadWAV(w);
		note("  chunk %p %u bytes", (void *)c, c ? (unsigned)c->alen : 0);
		if (c && nchunk < MAXKEPT)
			chunks[nchunk++] = c;
	} else {
		int raw = strncmp(w, "raw:", 4) == 0;
		int conv = strncmp(w, "conv:", 5) == 0;
		int key = strncmp(w, "key:", 4) == 0;
		const char *name = raw || key ? w + 4 : conv ? w + 5 : w;
		SDL_Surface *s = IMG_Load(name), *c;
		if (!s) {
			note("  IMG_Load: %s", SDL_GetError());
			return;
		}
		note("  %dx%d %s pitch %d", s->w, s->h, SDL_GetPixelFormatName(s->format->format), s->pitch);
		if (!raw && !key && screen) {
			c = SDL_ConvertSurface(s, screen->format, 0);
			note("  converted %p", (void *)c);
			if (c) {
				SDL_FreeSurface(s);
				s = c;
			}
		}
		if (!raw && !conv) {
			SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 0, 0, 0));
			note("  colour key set");
		}
		if (nsurf < MAXKEPT)
			surfaces[nsurf++] = s;
		else
			SDL_FreeSurface(s);
	}
}

int main(int argc, char **argv)
{
	int i;
	char word[256];

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '@') {
			FILE *fp = fopen(argv[i] + 1, "r");
			if (!fp) {
				note("can't open %s", argv[i] + 1);
				return 20;
			}
			while (fscanf(fp, "%255s", word) == 1)
				step(word);
			fclose(fp);
		} else {
			step(argv[i]);
		}
	}
	step("free");
	if (music)
		Mix_FreeMusic(music);
	Mix_CloseAudio();
	SDL_Quit();
	note("done");
	return 0;
}

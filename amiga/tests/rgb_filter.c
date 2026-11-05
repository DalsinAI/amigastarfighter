/* rgb_filter: IMG_Load_RW of one 8-bit RGB PNG (colour type 2) through
 * OpenGPU's SDL2_image, COUNT times.
 *
 *   RGBFilter [keep] [WIDTH [FILTER [COUNT]]]     (30 4 8)
 *   RGBFilter [keep] file NAME [COUNT]            a PNG file instead
 *
 *   keep: every surface stays until all COUNT are loaded (as a game keeps
 *   its sprites); without it each is freed before the next load.
 *
 *   WIDTH 30 (rows of 90 bytes, not a multiple of 4, as Starfighter's
 *   gfx/missileBoat1.png) or 32 (rows of 96); FILTER the PNG filter every
 *   row of the picture is stored with: 0 None, 1 Sub, 2 Up, 3 Average,
 *   4 Paeth. The pictures (rgb_pngs.h, from mkpng.py) are synthetic,
 *   31 rows high, and have no ancillary chunks.
 *
 * In the AmigaChrome lab (OpenGPU 0.6): "RGBFilter keep 30 4" and
 * "RGBFilter keep file gfx/missileBoat1.png" (from Work:Starfighter) take
 * a1200native down at the 3rd load (glibc "corrupted size vs. prev_size",
 * exit -6); keep 30 0/1/2/3, keep 32 4, and every run without keep, are
 * clean, and so is that picture stored as RGBA. See AMIGA_PORT.md.
 *
 * Each step goes to PROGDIR:rgbfilter.txt, opened and closed per line, so
 * it is on the disk after the emulator goes. Pixel (5,7) is checked
 * against what the picture holds (37,122,207).
 *
 * m68k-amigaos-gcc -noixemul -m68040 -m68881 -O2 rgb_filter.c \
 *     -o RGBFilter -I$SDL2KIT/include/SDL2 -L$SDL2KIT/lib \
 *     -lSDL2_image -lSDL2 -lm
 * The AmiCradle Team. GPL 3 or later, like Starfighter. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_image.h"
#include "rgb_pngs.h"

unsigned long __stack = 64 * 1024;

static void note(const char *fmt, ...)
{
	static int line;
	va_list ap;
	FILE *fp = fopen("PROGDIR:rgbfilter.txt", line++ ? "a" : "w");
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	putchar('\n');
	fflush(stdout);
	if (fp) {
		va_start(ap, fmt);
		vfprintf(fp, fmt, ap);
		va_end(ap);
		fputc('\n', fp);
		fclose(fp);
	}
}

int main(int argc, char **argv)
{
	const char *file = NULL;
	int w = 30, filter = 4, count = 8, n = 0, i, keep = 0;
	SDL_Surface *kept[64];

	if (argc > 1 && strcmp(argv[1], "keep") == 0) {
		keep = 1;
		argc--;
		argv++;
	}
	if (argc > 2 && strcmp(argv[1], "file") == 0) {
		file = argv[2];
		if (argc > 3)
			count = atoi(argv[3]);
	} else {
		if (argc > 1)
			w = atoi(argv[1]);
		if (argc > 2)
			filter = atoi(argv[2]);
		if (argc > 3)
			count = atoi(argv[3]);
		for (n = 0; n < (int)(sizeof pngs / sizeof pngs[0]); n++)
			if (pngs[n].w == w && pngs[n].filter == filter)
				break;
		if (n == (int)(sizeof pngs / sizeof pngs[0])) {
			printf("usage: %s [30|32 [0-4 [COUNT]]] | file NAME [COUNT]\n", argv[0]);
			return 5;
		}
	}
	if (count < 1 || count > 64)
		count = 8;
	if (file)
		note("%s, %d loads%s", file, count, keep ? ", kept" : "");
	else
		note("%dx%d RGB PNG, filter %d, %d bytes, %d loads%s", w, pngs[n].h, filter, pngs[n].size,
			count, keep ? ", kept" : "");

	for (i = 0; i < count; i++) {
		SDL_Surface *s;
		kept[i] = NULL;
		note("%d: IMG_Load", i);
		s = file ? IMG_Load(file) : IMG_Load_RW(SDL_RWFromConstMem(pngs[n].data, pngs[n].size), 1);
		if (!s) {
			note("%d: %s", i, SDL_GetError());
			continue;
		}
		if (s->format->BytesPerPixel == 4) {
			Uint8 r, g, b;
			SDL_GetRGB(((Uint32 *)((Uint8 *)s->pixels + 7 * s->pitch))[5], s->format, &r, &g, &b);
			note("%d: %dx%d %s pitch %d, pixel (5,7) %d,%d,%d", i, s->w, s->h,
				SDL_GetPixelFormatName(s->format->format), s->pitch, r, g, b);
		} else {
			note("%d: %dx%d %s pitch %d", i, s->w, s->h,
				SDL_GetPixelFormatName(s->format->format), s->pitch);
		}
		if (keep) {
			kept[i] = s;
		} else {
			SDL_FreeSurface(s);
			note("%d: freed", i);
		}
	}
	if (keep) {
		note("freeing");
		for (i = 0; i < count; i++)
			SDL_FreeSurface(kept[i]);
	}
	note("done");
	return 0;
}

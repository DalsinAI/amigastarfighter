/*
Copyright (C) 2003 Parallel Realities
Copyright (C) 2011, 2012, 2013 Guus Sliepen
Copyright (C) 2015-2020 The Diligent Circle <diligentcircle@riseup.net>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SDL.h"

#include "defs.h"
#include "structs.h"

#include "screen.h"
#include "window.h"

SDL_Renderer *renderer;
SDL_Texture *renderer_texture;

void renderer_update()
{
#ifdef AMIGA_TRACE
	static int traced;
	if (traced < 4)
		ATRACE("renderer_update %d: SDL_UpdateTexture pitch %d", traced, screen->pitch);
#endif
	SDL_UpdateTexture(renderer_texture, NULL, screen->pixels, screen->pitch);
#ifdef AMIGA_TRACE
	if (traced < 4)
		ATRACE("renderer_update %d: SDL_RenderCopy", traced);
#endif
	SDL_RenderCopy(renderer, renderer_texture, NULL, NULL);
#ifdef AMIGA_TRACE
	if (traced < 4)
		ATRACE("renderer_update %d: SDL_RenderPresent", traced);
#endif
	SDL_RenderPresent(renderer);
#ifdef AMIGA_TRACE
	if (traced < 4)
		ATRACE("renderer_update %d: done", traced);
	traced++;
#endif
}

// Call after screen_adjustDimensions.
void renderer_reset()
{
	if (renderer == NULL)
	{
		ATRACE("SDL_CreateRenderer");
		renderer = SDL_CreateRenderer(window, -1, 0);
#ifdef AMIGA_TRACE
		{
			SDL_RendererInfo ri;
			if (renderer != NULL && SDL_GetRendererInfo(renderer, &ri) == 0)
				ATRACE("renderer %s flags %x max texture %dx%d", ri.name, (unsigned)ri.flags,
					ri.max_texture_width, ri.max_texture_height);
		}
#endif
		if (renderer == NULL)
		{
			printf("Could not create renderer: %s\n", SDL_GetError());
			exit(1);
		}
	}
	ATRACE("SDL_RenderSetLogicalSize %dx%d", screen->w, screen->h);
	SDL_RenderSetLogicalSize(renderer, screen->w, screen->h);

	if (renderer_texture != NULL)
	{
		SDL_DestroyTexture(renderer_texture);
		renderer_texture = NULL;
	}

	ATRACE("SDL_CreateTexture ARGB8888 streaming %dx%d", screen->w, screen->h);
	renderer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screen->w, screen->h);
	if (renderer_texture == NULL)
	{
		printf("Couldn't create %ix%ix32 texture: %s\n", screen->w, screen->h, SDL_GetError());
		exit(1);
	}
}

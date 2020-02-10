#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "SDL.h"

static void GetPixel(FILE *file, unsigned char *pixels)
{
	static int fuck;
	static bool flipflop;

	if (!flipflop)
		fuck = fgetc(file);

	if (!flipflop)
	{
		*pixels++ = fuck & 0xF0;
		*pixels++ = fuck & 0xF0;
		*pixels++ = fuck & 0xF0;
	}
	else
	{
		*pixels++ = (fuck & 0xF) << 4;
		*pixels++ = (fuck & 0xF) << 4;
		*pixels++ = (fuck & 0xF) << 4;
	}

	flipflop = !flipflop;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *window = SDL_CreateWindow("view", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, 0);

	SDL_Surface *window_surface = SDL_GetWindowSurface(window);

	SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 256, 256, 0, SDL_PIXELFORMAT_RGB24);

	FILE *file = fopen("out", "rb");

	fseek(file, 0, SEEK_END);
	const size_t size = ftell(file);
	rewind(file);

	for (unsigned int i = 0; i < size / 32; ++i)
	{
		unsigned int x = i % 16;
		unsigned int y = i / 16;

		for (unsigned int h = 0; h < 8; ++h)
		{
			for (unsigned int w = 0; w < 8; ++w)
			{
				unsigned char *pixels = &((unsigned char*)surface->pixels)[(surface->pitch * (h + y * 8)) + ((w + x * 8) * 3)];
				GetPixel(file, pixels);
			}
		}
	}

	SDL_BlitScaled(surface, NULL, window_surface, NULL); // blit it to the screen

    SDL_UpdateWindowSurface(window);

	for (;;)
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT)
			break;
	}

	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

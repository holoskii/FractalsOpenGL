#pragma once
#include <SDL.h>

class MainWindow
{
	int max_count = 255;
	int color_mode = 0;
	float color_coefficient = 1.f;
	int xRes, yRes;
	float xPos = -2, yPos = -2;
	float xSize = 6, ySize = 3;

	Uint32* pixels;
	SDL_Window* sdl_window = NULL;
	SDL_Surface* sdl_surface = NULL;
	SDL_Renderer* sdl_renderer = NULL;
	SDL_Texture* sdl_texture = NULL;

	void update_pixels();
	void fill_lines(int i1, int i2, int(*mandelbort_fun)(float, float, int));
	void workLoop();
	int handle_event(SDL_Event event);

	void init_Window();
	void update_Window();
	void close_Window();
public:
	MainWindow(int x, int y);
};


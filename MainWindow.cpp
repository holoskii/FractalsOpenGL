#include "MainWindow.h"

#include <ctime>
#include <cmath>
#include <thread>
#include <iostream>

MainWindow::MainWindow(int x, int y) : xRes(x), yRes(y) {
	ySize = xSize * yRes / xRes;
	init_Window();
	workLoop();
	close_Window();
}

void MainWindow::update_pixels() {
	const int threads = 8;
	auto mandelbort_fun = [](float cx, float cy, const int max_count) -> int {

		int count = 0;
		float zx = 0, zy = 0, tempx;
		while ((zx * zx + zy * zy < 4) && (count < max_count))
		{
			tempx = zx * zx - zy * zy + cx;
			zy = 2 * zx * zy + cy;
			zx = tempx;
			++count;
		}
		return count;
	};

	std::thread t[threads];
	for (int i = 0; i < threads; ++i) {
		t[i] = std::thread(&MainWindow::fill_lines, this, i * yRes / threads, (i + 1) * yRes / threads, mandelbort_fun);
	}
	for (int i = 0; i < threads; ++i) {
		t[i].join();
	}

}

void MainWindow::fill_lines(int i1, int i2, int(*mandelbort_fun)(float, float, int)) {
	int val;
	if (color_mode == 0) {
		for (int i = i1; i < i2; i++) {
			for (int j = xRes - 1; j >= 0; j--) {
				val = mandelbort_fun(xPos + xSize * j / xRes, yPos + ySize * i / yRes, max_count);
				pixels[i * xRes + j] = 0x00010101 * (int)(pow(val / (float)max_count, color_coefficient) * 255);
			}
		}
	}
	else if (color_mode == 1) {
		for (int i = i1; i < i2; i++) {
			for (int j = xRes - 1; j >= 0; j--) {
				val = max_count - mandelbort_fun(xPos + xSize * j / xRes, yPos + ySize * i / yRes, max_count);
				pixels[i * xRes + j] = 0x00010101 * (int)(pow(val / (float)max_count, color_coefficient) * 255);
			}
		}
	}
	else if (color_mode == 2) {
		for (int i = i1; i < i2; i++) {
			for (int j = xRes - 1; j >= 0; j--) {
				val = mandelbort_fun(xPos + xSize * j / xRes, yPos + ySize * i / yRes, max_count);
				val = (int)(pow(val / (float)max_count, color_coefficient) * 255);
				pixels[i * xRes + j] = 0x00000101 * val;
			}
		}
	}
}

void MainWindow::workLoop()
{
	SDL_Event event;
	int begin_time = 0, time_taken = 0;
	int upd = 1, quit = 0;

	while (!quit)
	{
		begin_time = SDL_GetTicks();
		while (SDL_PollEvent(&event) == 1) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			else {
				upd |= handle_event(event);
			}
		}
		if (upd == 1) {
			update_pixels();
			upd = 0;
		}

		update_Window();

		time_taken = SDL_GetTicks() - begin_time;
		if (time_taken < 100) {
			SDL_Delay(100 - time_taken);
		}
		else {
			//std::cout << time_taken << endl;
		}
	}
}

int MainWindow::handle_event(SDL_Event event) {
	float sides = 1.f;
	int upd = 0;
	int state = 0;
	switch (event.type) {
	case SDL_KEYDOWN:
		upd = 1;
		switch (event.key.keysym.sym) {
		case SDLK_w:
		case SDLK_UP:
			yPos -= ySize / 10;
			break;
		case SDLK_s:
		case SDLK_DOWN:
			yPos += ySize / 10;
			break;
		case SDLK_d:
		case SDLK_RIGHT:
			xPos += xSize / 10;
			break;
		case SDLK_a:
		case SDLK_LEFT:
			xPos -= xSize / 10;
			break;
		case SDLK_q:
			xPos -= xSize * (1 - 1 / 1.25f) / 2;
			yPos -= ySize * (1 - 1 / 1.25f) / 2;
			xSize *= 1.25;
			ySize *= 1.25;
			break;
		case SDLK_e:
			xPos += xSize * (1 - 1 / 1.25f) / 2;
			yPos += ySize * (1 - 1 / 1.25f) / 2;
			xSize /= 1.25;
			ySize /= 1.25;
			break;
		case SDLK_m:
			color_coefficient += 0.1f;
			break;
		case SDLK_n:
			color_coefficient -= 0.1f;
			break;
		case SDLK_r:
			max_count *= 2;
			break;
		case SDLK_f:
			max_count /= 2;
			break;
		case SDLK_x:
			color_mode++;
			color_mode %= 3;
			break;
		default:
			//std::std::cout << "unknown key: " << event.key.keysym.sym << "\n";
			break;
		}
		break;
	case SDL_MOUSEWHEEL:
		upd = 1;
		if (event.wheel.y > 0)
		{
			xPos += xSize * (1 - 1 / 1.25f) / 2;
			yPos += ySize * (1 - 1 / 1.25f) / 2;
			xSize /= 1.25;
			ySize /= 1.25;
		}
		else if (event.wheel.y < 0)
		{
			xPos -= xSize * (1 - 1 / 1.25f) / 2;
			yPos -= ySize * (1 - 1 / 1.25f) / 2;
			xSize *= 1.25;
			ySize *= 1.25;
		}
		break;
	}
	return upd;
}

void MainWindow::init_Window() {
	srand((unsigned int)time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	sdl_window = SDL_CreateWindow("MyWindow", 50, 30, xRes, yRes, SDL_WINDOW_RESIZABLE);
	if (sdl_window == NULL) {
		std::cout << " > unable to open sdl_window\n";
		exit(EXIT_FAILURE);
	}
	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
	if (sdl_renderer == NULL) {
		std::cout << " > unable to open sdl_renderer\n";
		exit(EXIT_FAILURE);
	}
	SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 0, 255);
	SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_NONE);
	sdl_texture = SDL_CreateTexture(sdl_renderer,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, xRes, yRes);
	if (sdl_texture == NULL) {
		std::cout << " > unable to open sdl_texture\n";
		exit(EXIT_FAILURE);
	}
	pixels = new Uint32[xRes * yRes];
}

void MainWindow::update_Window() {
	SDL_UpdateTexture(sdl_texture, NULL, pixels, xRes * sizeof(Uint32));
	SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
	SDL_RenderPresent(sdl_renderer);
}

void MainWindow::close_Window() {
	delete[] pixels;
	SDL_DestroyTexture(sdl_texture);
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();
}
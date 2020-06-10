#pragma once
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <SDL.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

float xPos = -1, yPos = 0;
float xSize = 2, ySize = 3;

Uint32* pixels = NULL;
SDL_Window* sdl_window = NULL;
SDL_Surface* sdl_surface = NULL;
SDL_Renderer* sdl_renderer = NULL;
SDL_Texture* sdl_texture = NULL;

void MainWindow();
void update_pixels();
__global__ void kernel(Uint32* pixels, int* res, float* pos);
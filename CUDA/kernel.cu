#include "header.h"

int toTest = 1;
int kernPar1 = 6;
int kernPar2 = 256;

const int xRes = 1600, yRes = 900;

void MainWindow() {
	// initiation of window
	ySize = xSize * yRes / xRes;
	srand((unsigned int)time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	sdl_window = SDL_CreateWindow("MyWindow", 50, 30, xRes, yRes, SDL_WINDOW_RESIZABLE);
	if (sdl_window == NULL) {
		printf(" > unable to open sdl_window\n");
		exit(EXIT_FAILURE);
	}
	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
	if (sdl_renderer == NULL) {
		printf(" > unable to open sdl_renderer\n");
		exit(EXIT_FAILURE);
	}
	SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 0, 255);
	SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_NONE);
	sdl_texture = SDL_CreateTexture(sdl_renderer,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, xRes, yRes);
	if (sdl_texture == NULL) {
		printf(" > unable to open sdl_texture\n");
		exit(EXIT_FAILURE);
	}

	cudaMallocManaged(&pixels, sizeof(Uint32) * xRes * yRes);
	cudaDeviceSynchronize();

	// working loop
	SDL_Event event;
	int beginTime = 0, timeTaken = 0;
	int upd = 1, quit = 0;

	while (!quit)
	{
		beginTime = SDL_GetTicks();
		while (SDL_PollEvent(&event) == 1) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			else {
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
						xSize *= 1.25; ySize *= 1.25;
						break;
					case SDLK_e:
						xPos += xSize * (1 - 1 / 1.25f) / 2;
						yPos += ySize * (1 - 1 / 1.25f) / 2;
						xSize /= 1.25; ySize /= 1.25;
						break;
					default:
						break;
					}
					break;
				case SDL_MOUSEWHEEL:
					upd = 1;
					if (event.wheel.y > 0) {
						xPos += xSize * (1 - 1 / 1.25f) / 2;
						yPos += ySize * (1 - 1 / 1.25f) / 2;
						xSize /= 1.25; ySize /= 1.25;
					}
					else if (event.wheel.y < 0) {
						xPos -= xSize * (1 - 1 / 1.25f) / 2;
						yPos -= ySize * (1 - 1 / 1.25f) / 2;
						xSize *= 1.25; ySize *= 1.25;
					}
					break;
				}
			}
		}

		upd |= toTest;
		if (upd == 1) {
			update_pixels();
			upd = 0;
		}

		// update window
		SDL_UpdateTexture(sdl_texture, NULL, pixels, xRes * sizeof(Uint32));
		SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
		SDL_RenderPresent(sdl_renderer);

		int frameTime = 15;
		timeTaken = SDL_GetTicks() - beginTime;
		if (timeTaken < frameTime) {
			SDL_Delay(frameTime - timeTaken);
		}
		else {
			printf("Time of frame: %d\n", timeTaken);
		}
	}

	// close window
	cudaDeviceSynchronize();
	cudaFree(pixels);
	SDL_DestroyTexture(sdl_texture);
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();
}

void update_pixels() {
	int* res = NULL;
	float* pos = NULL;

	cudaMallocManaged(&res, sizeof(int) * 2);
	cudaMallocManaged(&pos, sizeof(float) * 2);

	res[0] = xRes; res[1] = yRes;

	pos[0] = xPos; pos[1] = yPos;
	pos[2] = xSize; pos[3] = ySize;

	kernel <<< kernPar1, kernPar2 >>> (pixels, res, pos);
	cudaFree(res);
	cudaFree(pos);
	cudaDeviceSynchronize();

	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		fprintf(stdout, "Failed to launch kernel (error code %s)!\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
}

__global__ void kernel(Uint32* pixels, int* res, float* pos) {
	int i, count, index, stride;
	float cx, cy, zx, zy, tempx;

	index = blockIdx.x * blockDim.x + threadIdx.x;
	stride = blockDim.x * gridDim.x;
	
	for (i = index; i < xRes * yRes; i += stride) {
		count = 0;
		zx = 0, zy = 0;
		cx = pos[0] + pos[2] * (i % res[0]) / res[0];
		cy = pos[1] + pos[3] * (i / res[0]) / res[1];

		while ((zx * zx + zy * zy < 4) && (count < 255)) {
			tempx = zx * zx - zy * zy + cx;
			zy = 2 * zx * zy + cy;
			zx = tempx;
			++count;
		}

		pixels[i] = 0x00010101 * count;
	}
}

int main(int argc, char** argv) {
	MainWindow();
	return EXIT_SUCCESS;
}


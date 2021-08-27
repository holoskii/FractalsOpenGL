#version 330 core
in vec4 gl_FragCoord;
out vec4 FragColor;

uniform vec2 pos = { -1.0f, -1.0f };				// position of bottom left point
uniform vec2 size = { -1.0f, -1.0f };				// screen dimensions in unit length
uniform vec2 viewportSize = { 1920.0f, 1080.0f };	// screen dimensions in pixels
uniform vec2 juliaConstant = { -0.8f, 0.156f };

#define MAX_ITER 0xFF

void main()
{
	// get point coordinate from screen coordinate
	float cx = pos.x + size.x * (gl_FragCoord.x / viewportSize.x);
	float cy = pos.y + size.y * (gl_FragCoord.y / viewportSize.y);
	float zx = cx, zy = cy;

	if (juliaConstant.x == -100.0f) {
		// Mandelbrot set
		zx = 0.0f;
		zy = 0.0f;
	}
	else {
		// Julia set
		cx = juliaConstant.x;
		cy = juliaConstant.y;
	}

	// get number of intersections
	int interations = 0;
	while ((zx * zx + zy * zy) < 4 && interations < MAX_ITER){
        float tempx = zx * zx - zy * zy + cx;
        zy = 2.0 * zx * zy + cy;
        zx = tempx;
        interations++;
    }

	// color points inside set in black
	if (interations == MAX_ITER) interations = 0;

	// normalize intersections value
	float finterations = (float(interations)) / MAX_ITER;

	// color set from yellow to red to black
	FragColor = vec4(pow(finterations, 0.75f), pow(finterations, 4), 0.1f * finterations, 1.0f);
}
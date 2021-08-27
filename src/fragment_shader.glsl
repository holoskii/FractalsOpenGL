#version 330 core
#define MAX_INTER 0xFF
in vec2 inCoord;

out vec4 FragColor;

uniform vec2 pos = { -1.0f, -1.0f };
uniform vec2 size = { -1.0f, -1.0f };

void main()
{
	int count = 0;
	float tempx;
	float zx = 0, zy = 0;
	float cx = pos.x + size.x * inCoord.x;
	float cy = pos.y + size.y * inCoord.y;

	while ((zx * zx + zy * zy < 4.0f) && (count < MAX_INTER)) {
		tempx = zx * zx - zy * zy + cx;
		zy = 2 * zx * zy + cy;
		zx = tempx;
		++count;
	}

	float fcount = count;
	fcount /= MAX_INTER;

	FragColor = vec4(vec3(fcount), 1.0f);
}
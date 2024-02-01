#version 410

#define maxIterations 8000

out vec4 color;

uniform dvec2 screenSize;
uniform dvec2 centerOffset;
uniform dvec2 moveOffset;
uniform dvec2 juliaC;
uniform double zoom;
uniform int mode;
uniform int colorMode;

vec3 getColor(int iterations, double zDot) {
	if (colorMode == 0) {
		float l = float(iterations) - log2(log2(float(zDot))) + 4.0;
		return 0.5 + 0.5 * cos(3.0 + l * 0.15 + vec3(0.0, 0.6, 1.0));
	} else {
		float t = iterations / float(maxIterations);
		float r = 9.0 * (1.0 - t) * t * t * t;
		float g = 15.0 * (1.0 - t) * (1.0 - t) * t * t;
		float b = 8.5 * (1.0 - t) * (1.0 - t) * (1.0 - t) * t;
		return vec3(r, g, b);
	}
}

vec3 mandelbrot(dvec2 c) {
	double c2 = dot(c, c);
	// skip computation inside M1 - https://iquilezles.org/articles/mset1bulb
	if( 256.0 * c2 * c2 - 96.0 * c2 + 32.0 * c.x - 3.0 < 0.0 ) return vec3(0);
	// skip computation inside M2 - https://iquilezles.org/articles/mset2bulb
	if( 16.0 * (c2 + 2.0 * c.x + 1.0) - 1.0 < 0.0 ) return vec3(0);

	dvec2 z = dvec2(0);
	double zDot;
	int i;
	// iterate
	for (i = 0; i < maxIterations; i++) {
		z = dvec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
		zDot = dot(z, z);
		if (zDot >= 256) break;
	}
	return getColor(i, zDot);
}

vec3 julia(dvec2 z) {
	double zDot;
	int i;
	// iterate
	for (i = 0; i < maxIterations; i++) {
		z = dvec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + juliaC;
		zDot = dot(z, z);
		if (zDot >= 256) break;
	}
	return getColor(i, zDot);
}

void main() {
	dvec2 ndc = (gl_FragCoord.xy) / min(screenSize.x, screenSize.y) * 2.0 - 1.0;
	dvec2 c = ndc / zoom + moveOffset + centerOffset;

	if (mode == 1) color = vec4(mandelbrot(c), 1);
	else           color = vec4(julia(c), 1);
}
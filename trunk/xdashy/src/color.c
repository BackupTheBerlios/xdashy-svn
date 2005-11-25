#include "color.h"
#include "color_bits.h"

Color color_rgb(float r, float g, float b) {
	Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = 1.0f;
	return c;
}

Color color_rgba(float r, float g, float b, float a) {
	Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
	return c;
}

uint32_t color_pack(Color c) {
	return PACK_COLOR32(c.a * 255.0f, c.r * 255.0f, c.g * 255.0f, c.b * 255.0f);
}

Color color_unpack(uint32_t pcol) {
	Color c;
	c.r = (pcol & RED_MASK32) >> RED_SHIFT32;
	c.g = (pcol & GREEN_MASK32) >> GREEN_SHIFT32;
	c.b = (pcol & BLUE_MASK32) >> BLUE_SHIFT32;
	c.a = (pcol & ALPHA_MASK32) >> ALPHA_SHIFT32;
	return c;
}

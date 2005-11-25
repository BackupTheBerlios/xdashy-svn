#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdint.h>

typedef struct Color {
	float r, g, b, a;
} Color;

Color color_rgb(float r, float g, float b);
Color color_rgba(float r, float g, float b, float a);

uint32_t color_pack(Color c);
Color color_unpack(uint32_t pcol);

#endif	/* _COLOR_H_ */

#ifndef MUSICTT_COLOR_H
#define MUSICTT_COLOR_H

#include "common.h"

typedef struct { uint8_t r, g, b; } Rgb;

/* Dominant, reasonably vivid colour of an RGBA32 image. */
Rgb color_dominant(const uint8_t *pixels, int width, int height, int pitch);

/* Background gradient and accent derived from the artwork colour. */
typedef struct {
    Rgb top;       /* dark tinted background */
    Rgb bottom;
    Rgb accent;    /* brighter version for controls */
    Rgb glow;      /* mid tone for the artwork halo */
} ColorTheme;

ColorTheme color_theme(Rgb base);

#endif

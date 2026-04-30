#ifndef COLOR_H
#define COLOR_H

#include <SDL3/SDL_surface.h>
#include <stdint.h>

typedef struct {
  uint32_t r;
  uint32_t g;
  uint32_t b;
} Color;

const static Color WHITE = {
    .r = 255,
    .g = 255,
    .b = 255,
};

const static Color RED = {
    .r = 255,
    .g = 0,
    .b = 0,
};
const static Color GREEN = {
    .r = 0,
    .g = 255,
    .b = 0,
};
const static Color BLUE = {
    .r = 0,
    .g = 0,
    .b = 255,
};
const static Color YELLOW = {
    .r = 0,
    .g = 255,
    .b = 255,
};

static inline void set_color(SDL_Surface *canvas, uint32_t x, uint32_t y,
                             const Color *color) {
  if (x > canvas->w || y > canvas->h) {
    return;
  }
  uint32_t *buffer = canvas->pixels;
  uint32_t offset = (canvas->h - y) * canvas->w + x;
  uint32_t mapped_color =
      SDL_MapSurfaceRGB(canvas, color->r, color->g, color->b);
  buffer[offset] = mapped_color;
}

#endif /* COLOR_H */

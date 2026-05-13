#ifndef COLOR_H
#define COLOR_H

#include "vec.h"
#include <SDL3/SDL_surface.h>
#include <stdint.h>

const static Vec3 WHITE = {
    .r = 255,
    .g = 255,
    .b = 255,
};

const static Vec3 RED = {
    .r = 255,
    .g = 0,
    .b = 0,
};
const static Vec3 GREEN = {
    .r = 0,
    .g = 255,
    .b = 0,
};
const static Vec3 BLUE = {
    .r = 0,
    .g = 0,
    .b = 255,
};
const static Vec3 YELLOW = {
    .r = 0,
    .g = 255,
    .b = 255,
};

static inline Vec3 random_color() {
  Vec3 color = {};
  color.r = rand() % 255;
  color.g = rand() % 255;
  color.b = rand() % 255;
  return color;
}

// TODO: oob access in this function
static inline void set_color(SDL_Surface *canvas, uint32_t x, uint32_t y,
                             const Vec3 *color) {
  if (x >= canvas->w || y >= canvas->h) {
    return;
  }
  uint32_t *buffer = (uint32_t *)canvas->pixels;
  uint32_t offset = (canvas->h - y) * canvas->w + x;
  uint32_t mapped_color =
      SDL_MapSurfaceRGB(canvas, color->r, color->g, color->b);
  memcpy((buffer + offset), &mapped_color, sizeof(uint32_t));
}

static inline void set_color_rgba(SDL_Surface *canvas, uint32_t x, uint32_t y,
                                  const Vec4 *color) {
  if (x > canvas->w || y > canvas->h) {
    return;
  }

  uint32_t *buffer = (uint32_t *)canvas->pixels;
  uint32_t offset = (canvas->h - y) * canvas->w + x;
  uint32_t mapped_color =
      SDL_MapSurfaceRGBA(canvas, color->r, color->g, color->b, color->a);
  buffer[offset] = mapped_color;
}

#endif /* COLOR_H */

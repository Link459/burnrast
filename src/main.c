#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#include <stdio.h>

#define BURNRAST_SDL_CHECK(x)                                                  \
  if (!x) {                                                                    \
    printf("%s\n", SDL_GetError());                                            \
  }

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

void update(SDL_Surface *canvas) {
  uint32_t *buffer = canvas->pixels;
  for (uint32_t y = 0; y < canvas->h; y++) {
    for (uint32_t x = 0; x < canvas->w; x++) {
      uint32_t offset = y * canvas->w + x;
      uint32_t color = SDL_MapSurfaceRGB(canvas, 0, 255, 255);
      buffer[offset] = color;
    }
  }
}

void set_color(SDL_Surface *canvas, uint32_t x, uint32_t y, Color color) {
  uint32_t *buffer = canvas->pixels;
  uint32_t offset = (canvas->h - y) * canvas->w + x;
  uint32_t mapped_color = SDL_MapSurfaceRGB(canvas, color.r, color.g, color.b);
  buffer[offset] = mapped_color;
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  uint32_t h = 64;
  uint32_t w = 64;
  SDL_Window *window = SDL_CreateWindow("burnrast", w, h, 0);

  SDL_Surface *canvas = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);

  SDL_Event event;
  bool run = true;

  SDL_LockSurface(canvas);

  SDL_ClearSurface(canvas, 0.0, 0.0, 0.0, 1.0);
  int ax = 7, ay = 3;
  int bx = 12, by = 37;
  int cx = 62, cy = 53;
  set_color(canvas, ax, ay, WHITE);
  set_color(canvas, bx, by, WHITE);
  set_color(canvas, cx, cy, WHITE);
  SDL_UnlockSurface(canvas);

  while (run) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        run = false;
        printf("Quit\n");
        break;
      default:
        break;
      }
    }

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    BURNRAST_SDL_CHECK(SDL_BlitSurface(canvas, 0, window_surface, 0))
    BURNRAST_SDL_CHECK(SDL_UpdateWindowSurface(window));
  }
  SDL_Quit();
  return 0;
}

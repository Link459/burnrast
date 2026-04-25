#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BURNRAST_SDL_CHECK(x)                                                  \
  if (!x) {                                                                    \
    printf("%s\n", SDL_GetError());                                            \
  }

#define BURNRAST_SWAP(a, b)                                                    \
  int32_t tmp_##a = a;                                                         \
  a = b;                                                                       \
  b = tmp_##a

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

void set_color(SDL_Surface *canvas, uint32_t x, uint32_t y,
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

void line(SDL_Surface *canvas, int32_t ax, int32_t ay, int32_t bx, int32_t by,
          const Color *color) {
  /*for (float t = 0.0; t < 1.0; t += 0.02) {
    int32_t new_x = round(ax + (bx - ax) * t);
    int32_t new_y = round(ay + (by - ay) * t);
    set_color(canvas, new_x, new_y, color);
  }*/

  bool steep = abs(ax - bx) < abs(ay - by);

  if (steep) {
    BURNRAST_SWAP(ax, ay);
    BURNRAST_SWAP(bx, by);
  }

  if (ax > bx) {
    BURNRAST_SWAP(ax, bx);
    BURNRAST_SWAP(ay, by);
  }

  float y = ay;
  for (int32_t x = ax; x <= bx; x++) {
    // float t = (x - ax) / (float)(bx - ax);
    // int32_t y = round(ay + (by - ay) * t);

    if (steep) {
      set_color(canvas, y, x, color);
    } else {
      set_color(canvas, x, y, color);
    }
    y += (by - ay) / (float)(bx - ax);
  }
}

void random_lines(SDL_Surface *canvas) {
  for (uint32_t i = 0; i < (1 << 12); i++) {
    int32_t ax = rand() % canvas->w;
    int32_t bx = rand() % canvas->w;
    int32_t ay = rand() % canvas->h;
    int32_t by = rand() % canvas->h;
    Color color = {
        .r = rand() % 255,
        .g = rand() % 255,
        .b = rand() % 255,
    };
    line(canvas, ax, ay, bx, by, &color);
  }
}

void project(const SDL_Surface *surface, const float *x, int32_t *res) {
  res[0] = (x[0] + 1.0) * surface->w / 2;
  res[1] = (x[1] + 1.0) * surface->h / 2;
}

void draw_model(SDL_Surface *canvas, const char *filepath) {
  uint32_t vertex_count = 0;
  uint32_t face_count = 0;

  FILE *file = fopen(filepath, "r");

  ssize_t read = 0;
  size_t len = 0;
  char *current_line = NULL;

  while ((read = getline(&current_line, &len, file) != -1)) {
    if (current_line[0] == 'v' && current_line[1] == ' ') {
      vertex_count++;
    } else if (current_line[0] == 'f') {
      face_count++;
    }
  }

  float *vertices = (float *)malloc(vertex_count * 3 * sizeof(float));
  int *face_vertices = (int *)malloc(face_count * 3 * sizeof(int));
  uint32_t current_vertex = 0;
  uint32_t current_face = 0;

  fseek(file, 0, SEEK_SET);
  while ((read = getline(&current_line, &len, file) != -1)) {
    if (current_line[0] == 'v' && current_line[1] == ' ') {
      char *new_line = current_line + 2;
      float pos0;
      float pos1;
      float pos2;
      if (sscanf(new_line, "%f %f %f", &pos0, &pos1, &pos2) != 3) {
        printf("Failed to parse line: %s", current_line);
        exit(0);
      }
      printf("%f %f %f\n", pos0, pos1, pos2);
      vertices[current_vertex + 0] = pos0;
      vertices[current_vertex + 1] = pos1;
      vertices[current_vertex + 2] = pos2;
      current_vertex += 3;
    } else if (current_line[0] == 'f' && current_line[1] == ' ') {
      char *new_line = current_line + 2;
      uint32_t pos[3];
      uint32_t unused[6];
      if (sscanf(new_line, "%u/%u/%u %u/%u/%u %u/%u/%u", &pos[0], &unused[0],
                 &unused[1], &pos[1], &unused[2], &unused[3], &pos[2],
                 &unused[4], &unused[5]) != 9) {
        printf("Failed to parse line: %s", current_line);
      }

      printf("face: %u %u %u\n", pos[0], pos[1], pos[2]);

      // -1 as .obj indices start at 1
      // * 3 as there are 3 floats per vertex/face index
      face_vertices[current_face + 0] = (pos[0] - 1) * 3;
      face_vertices[current_face + 1] = (pos[1] - 1) * 3;
      face_vertices[current_face + 2] = (pos[2] - 1) * 3;
      current_face += 3;
    }
  }

  printf("Vertex count: %d\n", vertex_count);
  printf("Face count: %d\n", face_count);

  fclose(file);

  for (uint32_t i = 0; i < face_count; i++) {
    float *a = &vertices[face_vertices[i * 3 + 0]];
    float *b = &vertices[face_vertices[i * 3 + 1]];
    float *c = &vertices[face_vertices[i * 3 + 2]];

    int32_t a_proj[2];
    int32_t b_proj[2];
    int32_t c_proj[2];
    project(canvas, a, a_proj);
    project(canvas, b, b_proj);
    project(canvas, c, c_proj);

    line(canvas, a_proj[0], a_proj[1], b_proj[0], b_proj[1], &RED);
    line(canvas, b_proj[0], b_proj[1], c_proj[0], c_proj[1], &RED);
    line(canvas, c_proj[0], c_proj[1], a_proj[0], a_proj[1], &RED);
  }

  free(vertices);
  free(face_vertices);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  uint32_t h = 640;
  uint32_t w = 640;
  SDL_Window *window = SDL_CreateWindow("burnrast", w, h, 0);

  SDL_Surface *canvas = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);

  SDL_LockSurface(canvas);

  SDL_ClearSurface(canvas, 0.0, 0.0, 0.0, 1.0);
  int ax = 7, ay = 3;
  int bx = 12, by = 37;
  int cx = 62, cy = 53;
  // set_color(canvas, ax, ay, &WHITE);
  // set_color(canvas, bx, by, &WHITE);
  // set_color(canvas, cx, cy, &WHITE);

  line(canvas, ax, ay, bx, by, &BLUE);
  line(canvas, cx, cy, bx, by, &GREEN);
  line(canvas, cx, cy, ax, ay, &YELLOW);
  line(canvas, ax, ay, cx, cy, &RED);

  draw_model(canvas, "assets/diablo3_pose.obj");
  // random_lines(canvas);
  SDL_UnlockSurface(canvas);

  SDL_Event event;
  bool run = true;
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

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "model.h"
#include "vec.h"

#define BURNRAST_SDL_CHECK(x)                                                  \
  if (!x) {                                                                    \
    printf("%s\n", SDL_GetError());                                            \
  }

#define BURNRAST_SWAP(a, b)                                                    \
  int32_t tmp_##a = a;                                                         \
  a = b;                                                                       \
  b = tmp_##a

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

Image z_buffer;
bool show_z_buffer = false;

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
  // int ierror = 0;
  for (int32_t x = ax; x <= bx; x++) {
    // float t = (x - ax) / (float)(bx - ax);
    // int32_t y = round(ay + (by - ay) * t);

    if (steep) {
      set_color(canvas, y, x, color);
    } else {
      set_color(canvas, x, y, color);
    }

    y += (by - ay) / (float)(bx - ax);
    // ierror += 2 * abs(by - ay);
    // y += (by > ay ? 1 : -1) * (ierror > bx - ax);
    // ierror -= 2 * abs(bx - ax) * (ierror > bx - ax);
  }
}

void triangle_outline(SDL_Surface *canvas, int32_t ax, int32_t ay, int32_t bx,
                      int32_t by, int32_t cx, int32_t cy, const Color *color) {
  line(canvas, ax, ay, bx, by, color);
  line(canvas, bx, by, cx, cy, color);
  line(canvas, cx, cy, ax, ay, color);
}

void triangle_scanline(SDL_Surface *canvas, int32_t ax, int32_t ay, int32_t bx,
                       int32_t by, int32_t cx, int32_t cy, const Color *color) {
  // Sort a, b & c such that a is the smallest
  if (ay > by) {
    BURNRAST_SWAP(ax, bx);
    BURNRAST_SWAP(ay, by);
  }
  if (ay > cy) {
    BURNRAST_SWAP(ax, cx);
    BURNRAST_SWAP(ay, cy);
  }
  if (by > cy) {
    BURNRAST_SWAP(cx, bx);
    BURNRAST_SWAP(cy, by);
  }

  int32_t total_height = cy - ay;

  // Rasterizes up to the boundary introduced by the midpoint  b
  if (ay != cy) {
    int32_t segment_height = by - ay;
    // Go over each segment
    for (int32_t y = ay; y <= by; y++) {
      // Evaluates x point32_t along the triangle boundary
      int32_t x1 = ax + ((cx - ax) * (y - ay)) / total_height;
      int32_t x2 = ax + ((bx - ax) * (y - ay)) / segment_height;

      // Draw a line from the leftmost x point32_t to the rightmost x point
      for (int32_t x = min(x1, x2); x < max(x1, x2); x++) {
        set_color(canvas, x, y, color);
      }
    }
  }

  // Do the same thing but from b to c
  if (by != cy) {
    int32_t segment_height = cy - by;
    for (int32_t y = by; y <= cy; y++) {
      int32_t x1 = ax + ((cx - ax) * (y - ay)) / total_height;
      int32_t x2 = bx + ((cx - bx) * (y - by)) / segment_height;

      for (int32_t x = min(x1, x2); x < max(x1, x2); x++) {
        set_color(canvas, x, y, color);
      }
    }
  }
}

float signed_triangle_area(int32_t ax, int32_t ay, int32_t bx, int32_t by,
                           int32_t cx, int32_t cy) {
  // 1/2 * g * h
  /*return 0.5f * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) +
                 (ay - cy) * (ax + cx));*/

  return 0.5f * ((ax - cx) * (by - ay) - (ax - bx) * (cy - ay));
}

void triangle_aabb(SDL_Surface *canvas, IVec3 a, IVec3 b, IVec3 c,
                   const Color *color) {
  float min_x = min(a.x, min(b.x, c.x));
  float min_y = min(a.y, min(b.y, c.y));
  float max_x = max(a.x, max(b.x, c.x));
  float max_y = max(a.y, max(b.y, c.y));

  float total_area = signed_triangle_area(a.x, a.y, b.x, b.y, c.x, c.y);

  // Backface culling
  if (total_area < 0.0f) {
    return;
  }

  for (int32_t x = min_x; x < max_x; x++) {
    for (int32_t y = min_y; y < max_y; y++) {
      float alpha = signed_triangle_area(x, y, b.x, b.y, c.x, c.y) / total_area;
      float beta = signed_triangle_area(x, y, c.x, c.y, a.x, a.y) / total_area;
      float gamma = signed_triangle_area(x, y, a.x, a.y, b.x, b.y) / total_area;
      if (alpha < 0 || beta < 0 || gamma < 0) {
        continue;
      }

      float z = (alpha * a.z + beta * b.z + gamma * c.z);

      float cmp_z;
      image_get(&z_buffer, x, y, &cmp_z);
      if (cmp_z >= z) {
        continue;
      }
      image_set(&z_buffer, x, y, &z);

      /*Color new_color = {
          .r = (alpha * BLUE.r + beta * GREEN.r + gamma * RED.r),
          .g = (alpha * BLUE.g + beta * GREEN.g + gamma * RED.g),
          .b = (alpha * BLUE.b + beta * GREEN.b + gamma * RED.b),
      };*/

      /*float k = min(alpha, min(beta, gamma));
      if (k > 0.1f) {
        continue;
      }*/
      if (show_z_buffer) {
        Color z_color = {z, z, z};
        set_color(canvas, x, y, &z_color);
      } else {

        set_color(canvas, x, y, color);
      }
    }
  }
}

void triangle(SDL_Surface *canvas, IVec3 a, IVec3 b, IVec3 c,
              const Color *color) {
  triangle_aabb(canvas, a, b, c, color);
  // triangle_scanline(canvas, ax, ay, bx, by, cx, cy, color);
  // triangle_outline(canvas, ax, ay, bx, by, cx, cy, color);
}

void random_lines(SDL_Surface *canvas) {
  for (uint32_t i = 0; i < (1 << 20); i++) {
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

void draw_test_triangle(SDL_Surface *canvas) {
  IVec3 a = {.x = 7, .y = 4, .z = 13};
  IVec3 b = {.x = 55, .y = 39, .z = 128};
  IVec3 c = {.x = 23, .y = 59, .z = 255};
  /*line(canvas, ax, ay, bx, by, &BLUE);
  line(canvas, cx, cy, bx, by, &GREEN);
  line(canvas, cx, cy, ax, ay, &YELLOW);
  line(canvas, ax, ay, cx, cy, &RED);*/

  triangle(canvas, a, b, c, &RED);
}

void draw_test_triangles(SDL_Surface *canvas) {
  // triangle(canvas, 7, 45, 35, 100, 45, 60, &RED);
  // triangle(canvas, 120, 35, 90, 5, 45, 110, &WHITE);
  // triangle(canvas, 115, 83, 80, 90, 85, 120, &GREEN);
}

IVec3 project(const SDL_Surface *surface, Vec3 x) {
  IVec3 res;
  res.x = (x.x + 1.0f) * surface->w / 2;
  res.y = (x.y + 1.0f) * surface->h / 2;
  res.z = (x.z + 1.0f) * 255.0f / 2;
  return res;
}

Vec3 persp(Vec3 v) {
  float c = 3.0f;
  float inv = 1.0 / (1.0 - v.z / c);

  Vec3 res = {};
  res.x = v.x * inv;
  res.y = v.y * inv;
  res.z = v.z * inv;
  return res;
}

Vec3 rot(const Vec3 *v) {
  float a = -M_PI / 6;
  Mat3 rotation;
  Vec3 top = {cosf(a), 0, sinf(a)};
  Vec3 mid = {0, 1, 0};
  Vec3 bottom = {-sinf(a), 0, cosf(a)};
  make_mat3(&top, &mid, &bottom, &rotation);
  return mat3_mul_vec(&rotation, v);
}

void draw_model(SDL_Surface *canvas, Model *model) {
  for (uint32_t i = 0; i < model->face_count; i++) {
    Vertex *a = &model->vertices[model->face_vertices[i * 3 + 0]];
    Vertex *b = &model->vertices[model->face_vertices[i * 3 + 1]];
    Vertex *c = &model->vertices[model->face_vertices[i * 3 + 2]];

    IVec3 a_proj = project(canvas, persp(rot(&a->position)));
    IVec3 b_proj = project(canvas, persp(rot(&b->position)));
    IVec3 c_proj = project(canvas, persp(rot(&c->position)));

    Color color = {a->color.x, a->color.y, a->color.z};
    triangle(canvas, a_proj, b_proj, c_proj, &color);
  }
}

int main() {

  SDL_Init(SDL_INIT_VIDEO);

  uint32_t h = 640;
  uint32_t w = 640;
  SDL_Window *window = SDL_CreateWindow("burnrast", w, h, 0);

  z_buffer = image_create(w, h, sizeof(float));

  SDL_Surface *canvas = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);

  // Model model = load_model("assets/diablo3_pose.obj");
  Model model = load_model("assets/african_head.obj");
  //  Model model = load_model("assets/boggie/body.obj");

  SDL_Event event;
  bool run = true;
  while (run) {
    run = true;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        run = false;
        break;
      case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_N) {
          show_z_buffer = !show_z_buffer;
        }
        break;
      default:
        break;
      }
    }

    float zero = 0.0;
    image_clear(&z_buffer, &zero);
    SDL_LockSurface(canvas);

    SDL_ClearSurface(canvas, 0.0, 0.0, 0.0, 1.0);

    draw_model(canvas, &model);

    draw_test_triangle(canvas);
    // draw_test_triangles(canvas);
    // random_lines(canvas);
    SDL_UnlockSurface(canvas);

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    BURNRAST_SDL_CHECK(SDL_BlitSurface(canvas, 0, window_surface, 0))
    BURNRAST_SDL_CHECK(SDL_UpdateWindowSurface(window));
  }

  free_model(&model);
  image_free(&z_buffer);
  SDL_Quit();
  return 0;
}

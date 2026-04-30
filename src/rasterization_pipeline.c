#include "rasterization_pipeline.h"

#include "color.h"
#include "model.h"
#include <stdlib.h>

Mat4 viewport(const int32_t x, const int32_t y, const int32_t w,
              const int32_t h) {
  Vec4 a = {w / 2.0, 0.0, 0.0, x + w / 2.0};
  Vec4 b = {0.0, h / 2.0, 0.0, y + h / 2.0};
  Vec4 c = {0.0, 0.0, 1.0, 0.0};
  Vec4 d = {0.0, 0.0, 0.0, 1.0};
  Mat4 viewport = {};
  make_mat4(&a, &b, &c, &d, &viewport);
  return viewport;
}

Mat4 perspective() {
  float f = 3.0f;
  Mat4 res = mat4_identity();
  res.data[2][3] = -1.0 / f;

  /*Vec4 top = {1.0f, 0.0f, 0.0f, 0.0f};
  Vec4 mid_top = {0.0f, 1.0f, 0.0, 0.0f};
  Vec4 mid_bottom = {0.0f, 0.0f, 1.0f, 0.0f};
  Vec4 bottom = {0.0f, 0.0f, -1.0f / f, 1.0f};
  make_mat4(&top, &mid_top, &mid_bottom, &bottom, &res);*/
  return res;
}

Mat4 look_at(const Vec3 *eye, const Vec3 *center, const Vec3 *up) {
  Vec3 diff = vec3_sub(eye, center);
  Vec3 n = vec3_normalize(&diff);
  Vec3 l = vec3_cross(up, &n);
  l = vec3_normalize(&l);

  Vec3 m = vec3_cross(&n, &l);
  m = vec3_normalize(&m);

  Vec4 top = {l.x, l.y, l.z, 0.0f};
  Vec4 mid_top = {m.x, m.y, m.z, 0.0f};
  Vec4 mid_bottom = {n.x, n.y, n.z, 0.0f};
  Vec4 bottom = {0.0f, 0.0f, 0.0f, 1.0f};
  Mat4 res = {};
  make_mat4(&top, &mid_top, &mid_bottom, &bottom, &res);
  return res;
}

Vec3 viewport_project(const SDL_Surface *surface, Vec3 x) {
  Vec3 res;
  res.x = (x.x + 1.0f) * surface->w / 2;
  res.y = (x.y + 1.0f) * surface->h / 2;
  res.z = (x.z + 1.0f) * 255.0f / 2;

  return res;
}

Vec3 persp(Vec3 v) {
  float f = 3.0f;
  float inv = 1.0 / (1.0 - v.z / f);

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

void create_rasterization_pipeline(uint32_t w, uint32_t h,
                                   RasterizationPipeline *pipeline) {
  Vec3 eye = {-1, 0, 2};
  Vec3 center = {0, 0, 0};
  Vec3 up = {0, 1, 0};

  pipeline->viewport = viewport(w / 16, h / 16, w * 7 / 8, h * 7 / 8);
  pipeline->projection = perspective();
  pipeline->view = look_at(&eye, &center, &up);
  pipeline->show_z_buffer = false;
  pipeline->z_buffer = image_create(w, h, sizeof(float));
  pipeline->canvas = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
  pipeline->topology = PRIMITIVE_TOPOLOGY_LINE;
}

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

float signed_triangle_area(int32_t ax, int32_t ay, int32_t bx, int32_t by,
                           int32_t cx, int32_t cy) {
  // 1/2 * g * h
  /*return 0.5f * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) +
                 (ay - cy) * (ax + cx));*/

  return 0.5f * ((ax - cx) * (by - ay) - (ax - bx) * (cy - ay));
}

void pipeline_triangle_aabb(RasterizationPipeline *pipeline, Vec3 a, Vec3 b,
                            Vec3 c, const Vertex *vertex_a,
                            const Vertex *vertex_b, const Vertex *vertex_c) {
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
      image_get(&pipeline->z_buffer, x, y, &cmp_z);
      if (cmp_z >= z) {
        continue;
      }
      image_set(&pipeline->z_buffer, x, y, &z);

      Color new_color = {
          .r = (alpha * vertex_a->color.r + beta * vertex_b->color.r +
                gamma * vertex_c->color.r),
          .g = (alpha * vertex_a->color.g + beta * vertex_b->color.g +
                gamma * vertex_c->color.g),
          .b = (alpha * vertex_a->color.b + beta * vertex_b->color.b +
                gamma * vertex_c->color.b),
      };

      Color new_uv = {.r = (alpha * vertex_a->uvw.x + beta * vertex_b->uvw.x +
                            gamma * vertex_c->uvw.x),
                      .g = (alpha * vertex_a->uvw.y + beta * vertex_b->uvw.y +
                            gamma * vertex_c->uvw.y),
                      .b = 0.0f};

      /*float k = min(alpha, min(beta, gamma));
      if (k > 0.1f) {
        continue;
      }*/
      if (pipeline->show_z_buffer) {
        Color z_color = {z, z, z};
        set_color(pipeline->canvas, x, y, &z_color);
      } else {
        set_color(pipeline->canvas, x, y, &new_color);
      }
    }
  }
}

#define BURNRAST_SWAP(a, b)                                                    \
  int32_t tmp_##a = a;                                                         \
  a = b;                                                                       \
  b = tmp_##a

void line(SDL_Surface *canvas, float ax, float ay, float bx, float by,
          const Color *color) {
  /*for (float t = 0.0; t < 1.0; t += 0.02) {
    int32_t new_x = round(ax + (bx - ax) * t);
    int32_t new_y = round(ay + (by - ay) * t);
    set_color(canvas, new_x, new_y, color);
  }*/

  bool steep = fabsf(ax - bx) < fabsf(ay - by);

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

void triangle_outline(RasterizationPipeline *pipeline, Vec3 a, Vec3 b, Vec3 c,
                      const Color *color) {
  line(pipeline->canvas, a.x, a.y, b.x, b.y, color);
  line(pipeline->canvas, b.x, b.y, c.x, c.y, color);
  line(pipeline->canvas, c.x, c.y, a.x, a.y, color);
}

void rasterize(RasterizationPipeline *pipeline, const Vec4 clip0,
               const Vec4 clip1, const Vec4 clip2, const Vertex *vertex_a,
               const Vertex *vertex_b, const Vertex *vertex_c) {
  Vec4 ndc[3] = {
      {
          clip0.x / clip0.w,
          clip0.y / clip0.w,
          clip0.z / clip0.w,
          1.0f, // clip0.w / clip0.w,
      },
      {
          clip1.x / clip1.w,
          clip1.y / clip1.w,
          clip1.z / clip1.w,
          1.0f, // clip1.w / clip1.w,
      },
      {
          clip2.x / clip2.w,
          clip2.y / clip2.w,
          clip2.z / clip2.w,
          1.0f, // clip2.w / clip2.w,
      },
  };

  Vec3 screen0 = viewport_project(pipeline->canvas, vec3_from_vec4(ndc[0]));
  Vec3 screen1 = viewport_project(pipeline->canvas, vec3_from_vec4(ndc[1]));
  Vec3 screen2 = viewport_project(pipeline->canvas, vec3_from_vec4(ndc[2]));

  /*ndc[0] = mat4_mul_vec(&pipeline->viewport, &ndc[0]);
  ndc[1] = mat4_mul_vec(&pipeline->viewport, &ndc[1]);
  ndc[2] = mat4_mul_vec(&pipeline->viewport, &ndc[2]);

  Vec3 screen0 = {ndc[0].x, ndc[0].y, ndc[0].z};
  Vec3 screen1 = {ndc[1].x, ndc[1].y, ndc[1].z};
  Vec3 screen2 = {ndc[2].x, ndc[2].y, ndc[2].z};*/

  if (pipeline->topology == PRIMITIVE_TOPOLOGY_TRIANGLE) {
    pipeline_triangle_aabb(pipeline, screen0, screen1, screen2, vertex_a,
                           vertex_b, vertex_c);
  } else if (pipeline->topology == PRIMITIVE_TOPOLOGY_LINE) {
    triangle_outline(pipeline, screen0, screen1, screen2, &RED);
  }
}

Vec4 apply_transform(const RasterizationPipeline *pipeline, Vec3 in) {
  Vec4 res = {
      in.x,
      in.y,
      in.z,
      1.0f,
  };

  /*Vec3 r = persp(in);
  res.x = r.x;
  res.y = r.y;
  res.z = r.z;*/

  // res = mat4_mul_vec(&pipeline->view, &res);
  res = mat4_mul_vec(&pipeline->projection, &res);
  return res;
}

void pipeline_draw(RasterizationPipeline *pipeline, const Model *model) {
  for (uint32_t i = 0; i < model->face_count; i++) {
    Vertex *vertex_a = &model->vertices[model->face_vertices[i * 3 + 0]];
    Vertex *vertex_b = &model->vertices[model->face_vertices[i * 3 + 1]];
    Vertex *vertex_c = &model->vertices[model->face_vertices[i * 3 + 2]];

    Vec4 clip_a = apply_transform(pipeline, vertex_a->position);
    Vec4 clip_b = apply_transform(pipeline, vertex_b->position);
    Vec4 clip_c = apply_transform(pipeline, vertex_c->position);

    rasterize(pipeline, clip_a, clip_b, clip_c, vertex_a, vertex_b, vertex_c);
  }
}

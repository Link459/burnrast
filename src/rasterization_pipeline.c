#include "rasterization_pipeline.h"

#include "color.h"
#include "core.h"
#include "model.h"

Mat4 viewport(const int32_t x, const int32_t y, const int32_t w,
              const int32_t h) {
  /*Vec4 a = {w / 2.0, 0.0, 0.0, x + w / 2.0};
  Vec4 b = {0.0, h / 2.0, 0.0, y + h / 2.0};
  Vec4 c = {0.0, 0.0, 1.0, 0.0};
  Vec4 d = {0.0, 0.0, 0.0, 1.0};
  Mat4 viewport = {};
  make_mat4(&a, &b, &c, &d, &viewport);
  return viewport;*/
  Mat4 viewport = glms_mat4_identity();
  viewport.m00 = w / 2.0;
  viewport.m13 = x + w / 2.0;

  viewport.m11 = h / 2.0;
  viewport.m13 = x + h / 2.0;
  return viewport;
}

Vec3 viewport_project(const SDL_Surface *surface, Vec3 x) {
  Vec3 res;
  res.x = (x.x + 1.0f) * surface->w / 2;
  res.y = (x.y + 1.0f) * surface->h / 2;
  res.z = (x.z + 1.0f) * 255.0f / 2;

  return res;
}

void create_rasterization_pipeline(
    uint32_t w, uint32_t h, const RasterizationPipelineCreateInfo *create_info,
    RasterizationPipeline *pipeline) {

  printf("Creating pipeline: %ux%u\n", w, h);

  pipeline->viewport = viewport(w / 16, h / 16, w * 7 / 8, h * 7 / 8);
  pipeline->show_z_buffer = false;
  pipeline->z_buffer = image_create(w, h, sizeof(float));
  pipeline->framebuffer = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
  pipeline->topology = create_info->topology;
  pipeline->vertex_shader = create_info->vertex_shader;
  pipeline->fragment_shader = create_info->fragment_shader;
}

void destroy_rasterization_pipeline(const RasterizationPipeline *pipeline) {
  SDL_DestroySurface(pipeline->framebuffer);
  image_free(&pipeline->z_buffer);
}

float signed_triangle_area(int32_t ax, int32_t ay, int32_t bx, int32_t by,
                           int32_t cx, int32_t cy) {
  // 1/2 * g * h
  /*return 0.5f * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) +
                 (ay - cy) * (ax + cx));*/

  return 0.5f * ((ax - cx) * (by - ay) - (ax - bx) * (cy - ay));
}

void pipeline_triangle_aabb(RasterizationPipeline *pipeline, Vec3 a, Vec3 b,
                            Vec3 c, const InterpolatedVertex *vertex_a,
                            const InterpolatedVertex *vertex_b,
                            const InterpolatedVertex *vertex_c) {
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

      Vec3 interpolated_color = {BURNRAST_INTERPOLATE3(
          vertex_a->color, vertex_b->color, vertex_c->color)};
      Vec2 interpolated_uv = {
          BURNRAST_INTERPOLATE2(vertex_a->uv, vertex_b->uv, vertex_c->uv)};
      Vec3 interpolated_normal = {BURNRAST_INTERPOLATE3(
          vertex_a->normal, vertex_b->normal, vertex_c->normal)};

      /*Vec3 interpolated_color = {
          .r = (alpha * vertex_a->color.r + beta * vertex_b->color.r +
                gamma * vertex_c->color.r),
          .g = (alpha * vertex_a->color.g + beta * vertex_b->color.g +
                gamma * vertex_c->color.g),
          .b = (alpha * vertex_a->color.b + beta * vertex_b->color.b +
                gamma * vertex_c->color.b),
      };

      Vec2 interpolated_uv = {
          .x = (alpha * vertex_a->uvw.x + beta * vertex_b->uvw.x +
                gamma * vertex_c->uvw.x),
          .y = (alpha * vertex_a->uvw.y + beta * vertex_b->uvw.y +
                gamma * vertex_c->uvw.y),
      };*/

      InterpolatedVertex interpolated = {};
      interpolated.color = interpolated_color;
      interpolated.uv = interpolated_uv;
      interpolated.normal = interpolated_normal;

      IVec2 frag_coord = {x, y};
      /*float k = min(alpha, min(beta, gamma));
      if (k > 0.1f) {
        continue;
      }*/
      if (pipeline->show_z_buffer) {
        Vec3 z_color = {z, z, z};
        set_color(pipeline->framebuffer, x, y, &z_color);
      } else {
        // set_color(pipeline->canvas, x, y, &interpolated.color);
        Vec3 frag_color = pipeline->fragment_shader(frag_coord, &interpolated);
        set_color(pipeline->framebuffer, x, y, &frag_color);
      }
    }
  }
}

void line(SDL_Surface *canvas, float ax, float ay, float bx, float by,
          const Vec3 *color) {
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
                      const Vec3 *color) {
  line(pipeline->framebuffer, a.x, a.y, b.x, b.y, color);
  line(pipeline->framebuffer, b.x, b.y, c.x, c.y, color);
  line(pipeline->framebuffer, c.x, c.y, a.x, a.y, color);
}

void rasterize(RasterizationPipeline *pipeline, const Vec4 clip0,
               const Vec4 clip1, const Vec4 clip2,
               const InterpolatedVertex *vertex_a,
               const InterpolatedVertex *vertex_b,
               const InterpolatedVertex *vertex_c) {
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

  Vec3 screen0 =
      viewport_project(pipeline->framebuffer, glms_vec3_make(&ndc[0].x));
  Vec3 screen1 =
      viewport_project(pipeline->framebuffer, glms_vec3_make(&ndc[1].x));
  Vec3 screen2 =
      viewport_project(pipeline->framebuffer, glms_vec3_make(&ndc[2].x));

  /*Vec3 screen0 =
      glms_mat4_mulv3(pipeline->viewport, glms_vec3_make(&ndc[0].x), 1.0f);
  Vec3 screen1 =
      glms_mat4_mulv3(pipeline->viewport, glms_vec3_make(&ndc[1].x), 1.0f);
  Vec3 screen2 =
      glms_mat4_mulv3(pipeline->viewport, glms_vec3_make(&ndc[2].x), 1.0f);*/

  if (pipeline->topology == PRIMITIVE_TOPOLOGY_TRIANGLE) {
    pipeline_triangle_aabb(pipeline, screen0, screen1, screen2, vertex_a,
                           vertex_b, vertex_c);
  } else if (pipeline->topology == PRIMITIVE_TOPOLOGY_LINE) {
    triangle_outline(pipeline, screen0, screen1, screen2, &RED);
  }
}

void pipeline_draw(RasterizationPipeline *pipeline, const Model *model) {
  for (uint32_t i = 0; i < model->face_count; i++) {
    // Vertex *vertex_a = &model->vertices[model->face_vertices[i * 3 + 0]];
    // Vertex *vertex_b = &model->vertices[model->face_vertices[i * 3 + 1]];
    // Vertex *vertex_c = &model->vertices[model->face_vertices[i * 3 + 2]];

    Vertex *vertex_a = &model->vertices[i * 3 + 0];
    Vertex *vertex_b = &model->vertices[i * 3 + 1];
    Vertex *vertex_c = &model->vertices[i * 3 + 2];

    InterpolatedVertex a = {};
    InterpolatedVertex b = {};
    InterpolatedVertex c = {};
    Vec4 clip_a = pipeline->vertex_shader(pipeline, vertex_a, &a);
    Vec4 clip_b = pipeline->vertex_shader(pipeline, vertex_b, &b);
    Vec4 clip_c = pipeline->vertex_shader(pipeline, vertex_c, &c);

    rasterize(pipeline, clip_a, clip_b, clip_c, &a, &b, &c);
  }
}

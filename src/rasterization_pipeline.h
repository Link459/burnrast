#ifndef RASTERIZATION_PIPELINE_H
#define RASTERIZATION_PIPELINE_H

#include "image.h"
#include "model.h"
#include "vec.h"
#include <SDL3/SDL_surface.h>

typedef struct {
  Mat4 viewport;
  Mat4 projection;
  Mat4 view;
  SDL_Surface *canvas;
  bool show_z_buffer;
  Image z_buffer;
} RasterizationPipeline;

void create_rasterization_pipeline(uint32_t w, uint32_t h,
                                   RasterizationPipeline *pipeline);
void pipeline_draw(RasterizationPipeline *pipeline, const Model *model);

Mat4 viewport(const int32_t x, const int32_t y, const uint32_t w,
              const uint32_t h);
Mat4 perspective();
Mat4 look_at(const Vec3 *eye, const Vec3 *center, const Vec3 *up);

float signed_triangle_area(int32_t ax, int32_t ay, int32_t bx, int32_t by,
                           int32_t cx, int32_t cy);

#endif /* RASTERIZATION_PIPELINE_H */

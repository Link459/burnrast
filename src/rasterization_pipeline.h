#ifndef RASTERIZATION_PIPELINE_H
#define RASTERIZATION_PIPELINE_H

#include "image.h"
#include "model.h"
#include "shader.h"
#include "vec.h"
#include <SDL3/SDL_surface.h>

typedef enum {
  PRIMITIVE_TOPOLOGY_LINE,
  PRIMITIVE_TOPOLOGY_TRIANGLE,
} PrimitiveTopology;

typedef struct {
  PrimitiveTopology topology;
  VertexShader vertex_shader;
  FragmentShader fragment_shader;
} RasterizationPipelineCreateInfo;

typedef struct RasterizationPipeline {
  Mat4 viewport;
  SDL_Surface *framebuffer;
  bool show_z_buffer;
  Image z_buffer;
  PrimitiveTopology topology;
  VertexShader vertex_shader;
  FragmentShader fragment_shader;
} RasterizationPipeline;

typedef struct InterpolatedVertex {
  Vec2 uv;
  Vec3 normal;
} InterpolatedVertex;
typedef InterpolatedVertex InterpolationVertex;

void create_rasterization_pipeline(
    uint32_t w, uint32_t h, const RasterizationPipelineCreateInfo *create_info,
    RasterizationPipeline *pipeline);
void destroy_rasterization_pipeline(const RasterizationPipeline *pipeline);

void pipeline_draw(RasterizationPipeline *pipeline, const Model *model);

float signed_triangle_area(int32_t ax, int32_t ay, int32_t bx, int32_t by,
                           int32_t cx, int32_t cy);

Vec3 viewport_project(const SDL_Surface *surface, Vec3 x);

#endif /* RASTERIZATION_PIPELINE_H */

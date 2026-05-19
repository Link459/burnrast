#ifndef SHADER_H
#define SHADER_H

#include "vec.h"

struct RasterizationPipeline;
struct InterpolatedVertex;
struct Vertex;

typedef struct {
  IVec2 frag_coord;
} FragmentInput;

typedef struct {
  float alpha;
  float beta;
  float gamma;
} InterpolationInput;

typedef Vec4 (*VertexShader)(void *vertex, void *interpolated_vertex);
typedef void (*InterpolationShader)(void *final, const void *a, const void *b,
                                  const void *c,
                                  const InterpolationInput *input);
typedef Vec3 (*FragmentShader)(void *vertex, const FragmentInput *input);

#endif /* SHADER_H */

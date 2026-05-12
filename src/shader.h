#ifndef SHADER_H
#define SHADER_H

#include "vec.h"

struct RasterizationPipeline;
struct InterpolatedVertex;
struct Vertex;

typedef Vec4 (*VertexShader)(const struct RasterizationPipeline *,
                             struct Vertex *vertex);
typedef Vec3 (*FragmentShader)(IVec2 frag_coord,
                               const struct InterpolatedVertex *v);

#endif /* SHADER_H */

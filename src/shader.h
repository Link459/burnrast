#ifndef SHADER_H
#define SHADER_H

#include "vec.h"

struct RasterizationPipeline;
struct Vertex;

typedef Vec4 (*VertexShader)(const struct RasterizationPipeline *,
                             struct Vertex *vertex);
typedef void (*FragmentShader)();

#endif /* SHADER_H */

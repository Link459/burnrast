#ifndef MODEL_H
#define MODEL_H

#include "color.h"
#include "vec.h"

#include <stdint.h>

typedef struct Vertex {
  Vec3 position;
  Vec3 uvw;
  Vec3 normal;
} Vertex;

typedef struct {
  Vertex *vertices;
  int *face_vertices;
  uint32_t vertex_count;
  uint32_t face_count;
} Model;

Model load_model(const char *filepath);
void model_free(Model *model);

#endif /* MODEL_H */

#ifndef MODEL_H
#define MODEL_H

#include "vec.h"

#include <stdint.h>

typedef struct {
  Vec3 position;
  Vec3 uvw;
  Vec3 normal;
  Vec3 color;
} Vertex;

typedef struct {
  Vertex *vertices;
  int *face_vertices;
  uint32_t vertex_count;
  uint32_t face_count;
} Model;

Model load_model(const char *filepath);
void free_model(Model *model);

#endif /* MODEL_H */

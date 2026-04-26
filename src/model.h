#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>

typedef struct {
  float *vertices;
  int *face_vertices;
  uint32_t vertex_count;
  uint32_t face_count;
} Model;

Model load_model(const char *filepath);
void free_model(Model *model);

#endif /* MODEL_H */

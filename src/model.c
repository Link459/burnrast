#include "model.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

bool is_position(const char *current) {
  return current[0] == 'v' && current[1] == ' ';
}

bool is_uv(const char *current) {
  return current[0] == 'v' && current[1] == 't';
}

bool is_normal(const char *current) {
  return current[0] == 'v' && current[1] == 'n';
}

#define BURNRAST_MODEL_ERROR()                                                 \
  do {                                                                         \
    printf("Failed to parse line: %s", current_line);                          \
    exit(-1);                                                                  \
  } while (1)

// TODO: figure out loading normals/uv coords
Model load_model(const char *filepath) {
  uint32_t vertex_count = 0;
  uint32_t face_count = 0;

  FILE *file = fopen(filepath, "r");

  ssize_t read = 0;
  size_t len = 0;
  char *current_line = NULL;

  uint32_t position_count;
  uint32_t normal_count;
  uint32_t uv_count;

  while ((read = getline(&current_line, &len, file) != -1)) {
    if (current_line[0] == 'v' && current_line[1] == ' ') {
      vertex_count++;
    } else if (current_line[0] == 'f') {
      face_count++;
    }
  }

  Vertex *vertices = malloc(vertex_count * sizeof(Vertex));

  Vec3 *positions = malloc(vertex_count * sizeof(Vec3));
  Vec3 *uv = malloc(vertex_count * sizeof(Vec3));
  Vec3 *normals = malloc(vertex_count * sizeof(Vec3));

  int *face_vertices = malloc(face_count * 3 * sizeof(int));
  uint32_t current_vertex_pos = 0;
  uint32_t current_vertex_uv = 0;
  uint32_t current_vertex_normal = 0;
  uint32_t current_face = 0;

  fseek(file, 0, SEEK_SET);
  while ((read = getline(&current_line, &len, file) != -1)) {
    if (is_position(current_line)) {
      char *new_line = current_line + 2;
      Vec3 position;
      if (sscanf(new_line, "%f %f %f", &position.x, &position.y, &position.z) !=
          3) {
        BURNRAST_MODEL_ERROR();
      }

      vertices[current_vertex_pos].position = position;
      Vec3 color = {};
      color.r = rand() % 255;
      color.g = rand() % 255;
      color.b = rand() % 255;
      vertices[current_vertex_pos].color = color;
      positions[current_vertex_pos] = position;
      current_vertex_pos++;
    } else if (is_uv(current_line)) {
      char *new_line = current_line + 3;
      Vec3 uvw;
      if (sscanf(new_line, "%f %f", &uvw.x, &uvw.y) != 2) {
        BURNRAST_MODEL_ERROR();
      }

      // vertices[current_vertex_uv].uvw = uvw;
      uv[current_vertex_uv] = uvw;
      current_vertex_uv++;
    } else if (is_normal(current_line)) {
      char *new_line = current_line + 3;
      Vec3 normal;
      if (sscanf(new_line, "%f %f %f", &normal.x, &normal.y, &normal.z) != 3) {
        BURNRAST_MODEL_ERROR();
      }

      // vertices[current_vertex_normal].normal = normal;
      normals[current_vertex_normal] = normal;
      current_vertex_normal++;
    } else if (current_line[0] == 'f' && current_line[1] == ' ') {
      char *new_line = current_line + 2;
      uint32_t pos[3];
      uint32_t uv[3];
      uint32_t normal[3];
      if (sscanf(new_line, "%u/%u/%u %u/%u/%u %u/%u/%u", &pos[0], &uv[0],
                 &normal[0], &pos[1], &uv[1], &normal[1], &pos[2], &uv[2],
                 &normal[2]) != 9) {
        BURNRAST_MODEL_ERROR();
      }

      // TODO: Set the vertex data up properly here

      // -1 as .obj indices start at 1
      // * 3 as there are 3 floats per vertex/face index
      face_vertices[current_face + 0] = (pos[0] - 1);
      face_vertices[current_face + 1] = (pos[1] - 1);
      face_vertices[current_face + 2] = (pos[2] - 1);
      current_face += 3;
    }
  }

  assert(current_vertex_pos == vertex_count);
  // assert(current_vertex_uv == vertex_count);
  // assert(current_vertex_normal == vertex_count);

  fclose(file);

  Model model = {
      .vertices = vertices,
      .face_vertices = face_vertices,
      .vertex_count = vertex_count,
      .face_count = face_count,
  };

  return model;
}

void model_free(Model *model) {
  free(model->vertices);
  free(model->face_vertices);
}

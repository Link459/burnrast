#include "model.h"
#include "core.h"

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

Model load_model(const char *filepath) {
  FILE *file = fopen(filepath, "r");
  if (!file) {
    printf("Failed to open path: %s\n", filepath);
        exit(-1);
  }

  ssize_t read = 0;
  size_t len = 0;
  char *current_line = NULL;

  uint32_t position_count = 0;
  uint32_t normal_count = 0;
  uint32_t uv_count = 0;


  uint32_t face_count = 0;
  while ((read = getline(&current_line, &len, file) != -1)) {
    if (is_position(current_line)) {
      position_count++;
    } else if (is_uv(current_line)) {
      uv_count++;
    } else if (is_normal(current_line)) {
      normal_count++;
    } else if (current_line[0] == 'f' && current_line[1] == ' ') {
      face_count++;
    }
  }

  uint32_t vertex_count =
      face_count * 3; // fmax(uv_count, fmax(position_count, normal_count));
  Vertex *vertices = malloc(vertex_count * sizeof(Vertex));

  Vec3 *positions = malloc(position_count * sizeof(Vec3));
  Vec3 *uvs = malloc(uv_count * sizeof(Vec3));
  Vec3 *normals = malloc(normal_count * sizeof(Vec3));

  int *face_vertices = malloc(face_count * 3 * sizeof(int));
  uint32_t current_vertex_pos = 0;
  uint32_t current_vertex_uv = 0;
  uint32_t current_vertex_normal = 0;
  uint32_t current_vertex = 0;

  fseek(file, 0, SEEK_SET);
  while ((read = getline(&current_line, &len, file) != -1)) {
    if (is_position(current_line)) {
      char *new_line = current_line + 2;
      Vec3 position;
      if (sscanf(new_line, "%f %f %f", &position.x, &position.y, &position.z) !=
          3) {
        BURNRAST_MODEL_ERROR();
      }

      positions[current_vertex_pos] = position;
      current_vertex_pos++;
    } else if (is_uv(current_line)) {
      char *new_line = current_line + 3;
      Vec3 uvw;
      if (sscanf(new_line, "%f %f", &uvw.x, &uvw.y) != 2) {
        BURNRAST_MODEL_ERROR();
      }

      // vertices[current_vertex_uv].uvw = uvw;
      uvs[current_vertex_uv] = uvw;
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
      vertices[current_vertex + 0].position = positions[pos[0] - 1];
      vertices[current_vertex + 1].position = positions[pos[1] - 1];
      vertices[current_vertex + 2].position = positions[pos[2] - 1];

      vertices[current_vertex + 0].uvw = uvs[uv[0] - 1];
      vertices[current_vertex + 1].uvw = uvs[uv[1] - 1];
      vertices[current_vertex + 2].uvw = uvs[uv[2] - 1];

      vertices[current_vertex + 0].normal = normals[normal[0] - 1];
      vertices[current_vertex + 1].normal = normals[normal[1] - 1];
      vertices[current_vertex + 2].normal = normals[normal[2] - 1];

      face_vertices[current_vertex + 0] = current_vertex + 0;
      face_vertices[current_vertex + 1] = current_vertex + 1;
      face_vertices[current_vertex + 2] = current_vertex + 2;
      current_vertex += 3;
    }
  }

  assert(current_vertex == vertex_count);

  // assert(current_vertex_pos == vertex_count);
  // assert(current_vertex_uv == vertex_count);
  // assert(current_vertex_normal == vertex_count);

  fclose(file);
  free(positions);
  free(normals);
  free(uvs);

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

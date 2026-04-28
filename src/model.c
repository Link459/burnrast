#include "model.h"

#include <stdio.h>
#include <stdlib.h>

Model load_model(const char *filepath) {
  uint32_t vertex_count = 0;
  uint32_t face_count = 0;

  FILE *file = fopen(filepath, "r");

  ssize_t read = 0;
  size_t len = 0;
  char *current_line = NULL;

  while ((read = getline(&current_line, &len, file) != -1)) {
    if (current_line[0] == 'v' && current_line[1] == ' ') {
      vertex_count++;
    } else if (current_line[0] == 'f') {
      face_count++;
    }
  }

  Vertex *vertices = malloc(vertex_count * sizeof(Vertex));
  int *face_vertices = malloc(face_count * 3 * sizeof(int));
  uint32_t current_vertex_pos = 0;
  uint32_t current_vertex_uv = 0;
  uint32_t current_vertex_normal = 0;
  uint32_t current_face = 0;

  fseek(file, 0, SEEK_SET);
  while ((read = getline(&current_line, &len, file) != -1)) {
    if (current_line[0] == 'v' && current_line[1] == ' ') {
      char *new_line = current_line + 2;
      Vec3 position;
      if (sscanf(new_line, "%f %f %f", &position.x, &position.y, &position.z) !=
          3) {
        printf("Failed to parse line: %s", current_line);
        exit(0);
      }

      vertices[current_vertex_pos].position = position;
      Vec3 color = {};
      color.x = rand() % 255;
      color.y = rand() % 255;
      color.z = rand() % 255;
      vertices[current_vertex_pos].color = color;
      current_vertex_pos++;
    } else if (current_line[0] == 'v' && current_line[1] == 't') {
      char *new_line = current_line + 3;
      Vec3 uvw;
      if (sscanf(new_line, "%f %f", &uvw.x, &uvw.y) != 2) {
        printf("Failed to parse line: %s", current_line);
        exit(0);
      }

      vertices[current_vertex_uv].uvw = uvw;
      current_vertex_uv++;
    } else if (current_line[0] == 'v' && current_line[1] == 'n') {
      char *new_line = current_line + 3;
      Vec3 normal;
      if (sscanf(new_line, "%f %f %f", &normal.x, &normal.y, &normal.z) != 3) {
        printf("Failed to parse line: %s", current_line);
        exit(0);
      }

      vertices[current_vertex_normal].normal = normal;
      current_vertex_normal++;
    } else if (current_line[0] == 'f' && current_line[1] == ' ') {
      char *new_line = current_line + 2;
      uint32_t pos[3];
      uint32_t unused[6];
      if (sscanf(new_line, "%u/%u/%u %u/%u/%u %u/%u/%u", &pos[0], &unused[0],
                 &unused[1], &pos[1], &unused[2], &unused[3], &pos[2],
                 &unused[4], &unused[5]) != 9) {
        printf("Failed to parse line: %s", current_line);
      }

      // printf("face: %u %u %u\n", pos[0], pos[1], pos[2]);

      // -1 as .obj indices start at 1
      // * 3 as there are 3 floats per vertex/face index
      face_vertices[current_face + 0] = (pos[0] - 1);
      face_vertices[current_face + 1] = (pos[1] - 1);
      face_vertices[current_face + 2] = (pos[2] - 1);
      current_face += 3;
    }
  }

  printf("Vertex count: %d\n", vertex_count);
  printf("Face count: %d\n", face_count);

  fclose(file);

  Model model = {
      .vertices = vertices,
      .face_vertices = face_vertices,
      .vertex_count = vertex_count,
      .face_count = face_count,
  };

  return model;
}

void free_model(Model *model) {
  free(model->vertices);
  free(model->face_vertices);
}

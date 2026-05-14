#ifndef IMAGE_H
#define IMAGE_H

#include "vec.h"
#include <stdint.h>

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  uint8_t *data;
} Image;

Image image_create(uint32_t width, uint32_t height, uint32_t stride);
void image_free(const Image *image);

void image_clear(Image *image, void *value);
void image_set(Image *image, uint32_t x, uint32_t y, void *value);
void image_get(const Image *image, uint32_t x, uint32_t y, void *value);

Image image_load(const char *path);
Vec3 image_sample(const Image *image, Vec2 uv);
Vec3 image_sample_normal(const Image *image, Vec2 uv);

#endif /* IMAGE_H */

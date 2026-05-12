#include "image.h"

#include <stdlib.h>
#include <string.h>

Image image_create(uint32_t width, uint32_t height, uint32_t stride) {
  uint32_t size = width * height * stride;
  uint8_t *data = malloc(size);
  Image image = {
      .width = width,
      .height = height,
      .stride = stride,
      .data = data,
  };

  return image;
}

void image_free(const Image *image) { free(image->data); }

void image_clear(Image *image, void *value) {
  for (uint32_t x = 0; x < image->width; x++) {
    for (uint32_t y = 0; y < image->height; y++) {
      image_set(image, x, y, value);
    }
  }
}

void image_set(Image *image, uint32_t x, uint32_t y, void *value) {
  if (x >= image->width || y >= image->height || x < 0 || y < 0) {
    return;
  }
  // uint32_t offset = (image->height - y) * image->width + x;
  uint32_t offset = x + y * image->width;

  memcpy(image->data + (offset * image->stride), value, image->stride);
}

void image_get(Image *image, uint32_t x, uint32_t y, void *value) {
  if (x >= image->width || y >= image->height || x < 0 || y < 0) {
    return;
  }
  // uint32_t offset = (image->height - y) * image->width + x;
  uint32_t offset = x + y * image->width;
  memcpy(value, image->data + (offset * image->stride), image->stride);
}

#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  uint8_t *data;
} Image;

Image image_create(uint32_t width, uint32_t height, uint32_t stride);
void image_free(Image *image);

void image_clear(Image *image, void *value);
void image_set(Image *image, uint32_t x, uint32_t y, void *value);
void image_get(Image *image, uint32_t x, uint32_t y, void *value);

#endif /* IMAGE_H */

#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  uint8_t *data;
} Image;

void image_set(Image *image, uint32_t x, uint32_t y, float value);

#endif /* IMAGE_H */

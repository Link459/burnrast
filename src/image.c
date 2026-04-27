#include "image.h"

void image_set(Image *image, uint32_t x, uint32_t y, float value) {
  if (x > image->width || y > image->height) {
    return;
  }
  uint32_t offset = (image->height - y) * image->width + x;
  image->data[offset * image->stride] = value;
}

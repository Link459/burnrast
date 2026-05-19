#ifndef CORE_H
#define CORE_H

#define BURNRAST_SDL_CHECK(x)                                                  \
  if (!x) {                                                                    \
    printf("%s\n", SDL_GetError());                                            \
  }

#define BURNRAST_SWAP(a, b)                                                    \
  do {                                                                         \
    int32_t tmp_##a = a;                                                       \
    a = b;                                                                     \
    b = tmp_##a;                                                               \
  } while (1)

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#define BURNRAST_MODEL_ERROR()                                                 \
  do {                                                                         \
    printf("Failed to parse line: %s", current_line);                          \
    exit(-1);                                                                  \
  } while (1)

#define BURNRAST_INTERPOLATE2(input, a, b, c)                                  \
  input->alpha *a.x + input->beta *b.x + input->gamma *c.x,                           \
      input->alpha *a.y + input->beta *b.y + input->gamma *c.y,

#define BURNRAST_INTERPOLATE3(input, a, b, c)                                  \
  input->alpha *a.x + input->beta *b.x + input->gamma *c.x,                    \
      input->alpha *a.y + input->beta *b.y + input->gamma *c.y,                \
      input->alpha *a.z + input->beta *b.z + input->gamma *c.z,

#endif /* CORE_H */

#ifndef VEC_H
#define VEC_H

#include <stdint.h>

typedef struct {
  int32_t x;
  int32_t y;
  int32_t z;
} IVec3;

typedef struct {
  float x;
  float y;
  float z;
} Vec3;

/*inline Vec3 vec3_add_scalar(const Vec3 *vec, float v) {
  return Vec3{vec->x + v, vec->y + v, vec->z + v};
}
inline Vec3 vec3_sub_scalar(const Vec3 *vec, float v) {
  return Vec3{vec->x - v, vec->y - v, vec->z - v};
}
inline Vec3 vec3_add(const Vec3 *a, const Vec3 *b) {
  return Vec3{a->x + b->x, a->y + b->y, a->z + b->z};
}
inline Vec3 vec3_sub(const Vec3 *a, const Vec3 *b) {
  return Vec3{a->x - b->x, a->y - b->y, a->z - b->z};
}*/

typedef struct {
  float x;
  float y;
  float z;
  float w;
} Vec4;

/*inline Vec4 vec4_add_scalar(const Vec4 *vec, float v) {
  return Vec4{vec->x + v, vec->y + v, vec->z + v, vec->w + v};
}
inline Vec4 vec4_sub_scalar(const Vec4 *vec, float v) {
  return Vec4{vec->x - v, vec->y - v, vec->z - v, vec->w - v};
}
inline Vec4 vec4_add(const Vec4 *a, const Vec4 *b) {
  return Vec4{a->x + b->x, a->y + b->y, a->z + b->z, a->w + b->w};
}
inline Vec4 vec4_sub(const Vec4 *a, const Vec4 *b) {
  return Vec4{a->x - b->x, a->y - b->y, a->z - b->z, a->w + b->w};
}*/

typedef struct {
  float data[4][4];
} Mat4;

inline Vec4 mat4_mul_vec(const Mat4 *mat, Vec4 *v) {
  Vec4 res = {};
  res.x = mat->data[0][0] * v->x + mat->data[1][0] * v->y +
          mat->data[2][0] * v->z + mat->data[3][0] * v->w;
  res.y = mat->data[0][1] * v->x + mat->data[1][1] * v->y +
          mat->data[2][1] * v->z + mat->data[3][1] * v->w;
  res.z = mat->data[0][2] * v->x + mat->data[1][2] * v->y +
          mat->data[2][2] * v->z + mat->data[3][2] * v->w;
  res.w = mat->data[0][3] * v->x + mat->data[1][3] * v->y +
          mat->data[2][3] * v->z + mat->data[3][3] * v->w;
  return res;
}

typedef struct {
  float data[3][3];
} Mat3;

static inline void make_mat3(const Vec3 *a, const Vec3 *b, const Vec3 *c, Mat3 *res) {
  res->data[0][0] = a->x;
  res->data[0][1] = a->y;
  res->data[0][2] = a->z;

  res->data[1][0] = b->x;
  res->data[1][1] = b->y;
  res->data[1][2] = b->z;

  res->data[2][0] = c->x;
  res->data[2][1] = c->y;
  res->data[2][2] = c->z;
}

static inline Vec3 mat3_mul_vec(const Mat3 *mat, const Vec3 *v) {
  Vec3 res = {};
  res.x =
      mat->data[0][0] * v->x + mat->data[1][0] * v->y + mat->data[2][0] * v->z;
  res.y =
      mat->data[0][1] * v->x + mat->data[1][1] * v->y + mat->data[2][1] * v->z;
  res.z =
      mat->data[0][2] * v->x + mat->data[1][2] * v->y + mat->data[2][2] * v->z;
  return res;
}

#endif /* VEC_H */

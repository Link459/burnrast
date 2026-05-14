#include <SDL3/SDL_init.h>
#include <SDL3/SDL_time.h>
#include <SDL3/SDL_video.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "image.h"
#include "model.h"
#include "rasterization_pipeline.h"
#include "vec.h"

#define BURNRAST_SDL_CHECK(x)                                                  \
  if (!x) {                                                                    \
    printf("%s\n", SDL_GetError());                                            \
  }

#define BURNRAST_SWAP(a, b)                                                    \
  int32_t tmp_##a = a;                                                         \
  a = b;                                                                       \
  b = tmp_##a

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

Vec3 persp(Vec3 v) {
  float f = 3.0f;
  float inv = 1.0 / (1.0 - v.z / f);

  Vec3 res = {};
  res.x = v.x * inv;
  res.y = v.y * inv;
  res.z = v.z * inv;
  return res;
}

Vec3 rot(const Vec3 v) {
  float a = M_PI / 6;
  /*Mat3 rotation;
  Vec3 top = {cosf(a), 0, sinf(a)};
  Vec3 mid = {0, 1, 0};
  Vec3 bottom = {-sinf(a), 0, cosf(a)};
  glms_mat3_make([ top, mid, bottom, rotation ]);
  return glms_mat4_mulv(&rotation, v);*/
  Vec3 axis = {0.0, 1.0, 0.0};
  return glms_mat4_mulv3(glms_rotate(glms_mat4_identity(), a, axis), v, 1.0);
}

Mat4 perspective() {
  float f = 3.0f;
  Mat4 res = glms_mat4_identity();
  // res.data[2][3] = -1.0 / f;
  res.m23 = -1.0 / f;

  /*Vec4 top = {1.0f, 0.0f, 0.0f, 0.0f};
  Vec4 mid_top = {0.0f, 1.0f, 0.0, 0.0f};
  Vec4 mid_bottom = {0.0f, 0.0f, 1.0f, 0.0f};
  Vec4 bottom = {0.0f, 0.0f, -1.0f / f, 1.0f};
  make_mat4(&top, &mid_top, &mid_bottom, &bottom, &res);*/
  return res;
}

Mat4 look_at(const Vec3 *eye, const Vec3 *center, const Vec3 *up) {
  /*Vec3 diff = vec3_sub(eye, center);
  Vec3 n = vec3_normalize(&diff);
  Vec3 l = vec3_cross(up, &n);
  l = vec3_normalize(&l);

  Vec3 m = vec3_cross(&n, &l);
  m = vec3_normalize(&m);

  Vec4 top = {l.x, l.y, l.z, 0.0f};
  Vec4 mid_top = {m.x, m.y, m.z, 0.0f};
  Vec4 mid_bottom = {n.x, n.y, n.z, 0.0f};
  Vec4 bottom = {0.0f, 0.0f, 0.0f, 1.0f};
  Mat4 res = {};
  make_mat4(&top, &mid_top, &mid_bottom, &bottom, &res);
  return res;*/
  return glms_lookat(*eye, *center, *up);
}

Mat4 projection = {};
Mat4 view = {};
Mat4 transform = {};

uint32_t vert = 0;
Vec3 tris[3];

Vec4 simple_vertex_shader(const struct RasterizationPipeline *pipeline,
                          struct Vertex *vertex) {
  Vec4 res = {
      vertex->position.x,
      vertex->position.y,
      vertex->position.z,
      1.0f,
  };

  res = glms_mat4_mulv(transform, res);
  //  res = glms_mat4_mulv(pipeline->view, res);
  res = glms_mat4_mulv(projection, res);

  tris[vert] = glms_vec3_make(&res.x);
  vert = (vert + 1) % 3;

  return res;
}

Vec3 simple_fragment_shader(IVec2 frag_coord, const InterpolatedVertex *v) {

  Vec3 ab = glms_vec3_sub(tris[1], tris[0]);
  Vec3 ac = glms_vec3_sub(tris[2], tris[1]);
  Vec3 normal = glms_normalize(glms_cross(ab, ac));
  /*Vec3 res = {};
  res.x = v->uv.x * 255.0f;
  res.y = v->uv.y * 255.0f;
  res.z = 0.0f;*/

  float ambient = 0.3f;
  Vec3 l = {1.0f, 1.0f, 1.0f};
  l = glms_normalize(l);
  Vec3 c = {0.0f, 0.0f, 1.0f};
  float NoL = glms_dot(normal, l);
  float diffuse = fmax(0.0, NoL);

  // Vec3 r = glms_vec3_sub(glms_vec3_scale(normal, 2.0f * NoL), l);
  Vec3 r = glms_vec3_reflect(glms_vec3_negate(l), normal);
  float e = 35.0f;
  // float specular = powf(glms_vec3_dot(c, r), e);
  float specular = powf(max(0.0, r.z), e);

  if (specular >= 1.0f) {
    printf("a: %f,d: %f,s: %f\n", ambient, diffuse, specular);
  }
  Vec3 res =
      glms_vec3_scale(v->color, fmin(1.0f, ambient + 0.4 * diffuse + specular));
  return res;
  // return glms_vec3_scale(res, 255.0f);
  // seturn res;
  // return glms_vec3_scale(v->normal, 255.0f);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  uint32_t h = 640;
  uint32_t w = 640;
  SDL_Window *window = SDL_CreateWindow("burnrast", w, h, 0);

  // Model model = load_model("assets/diablo3_pose.obj");
  Model model = load_model("assets/african_head.obj");
  // Model model = load_model("assets/boggie/body.obj");

  RasterizationPipelineCreateInfo create_info = {
      .topology = PRIMITIVE_TOPOLOGY_TRIANGLE,
      .vertex_shader = simple_vertex_shader,
      .fragment_shader = simple_fragment_shader,
  };
  RasterizationPipeline pipeline = {};
  create_rasterization_pipeline(w, h, &create_info, &pipeline);

  projection = perspective();
  Vec3 eye = {-1, 0, 2};
  Vec3 center = {0, 0, 0};
  Vec3 up = {0, 1, 0};
  view = look_at(&eye, &center, &up);
  float a = M_PI / 6;
  Vec3 axis = {0.0, 1.0, 0.0};

  SDL_Time current_time = 0;
  SDL_Time previous_time = 0;
  SDL_GetCurrentTime(&current_time);
  float average_fps = 0.0f;
  uint32_t frames = 0;

  SDL_Event event;
  bool run = true;
  while (run) {
    run = true;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        run = false;
        break;
      case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_N) {
          pipeline.show_z_buffer = !pipeline.show_z_buffer;
        } else if (event.key.key == SDLK_F) {
          static bool f = false;
          f = !f;
          SDL_SetWindowFullscreen(window, f);
        } else if (event.key.key == SDLK_P) {
          if (create_info.topology == PRIMITIVE_TOPOLOGY_LINE) {
            create_info.topology = PRIMITIVE_TOPOLOGY_TRIANGLE;
          } else {
            create_info.topology = PRIMITIVE_TOPOLOGY_LINE;
          }
          create_rasterization_pipeline(w, h, &create_info, &pipeline);
        }
        break;
      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
        if (w == event.window.data1 && h == event.window.data2) {
          break;
        }

        destroy_rasterization_pipeline(&pipeline);
        w = event.window.data1;
        h = event.window.data2;
        create_rasterization_pipeline(w, h, &create_info, &pipeline);
        break;
      }
      default:
        break;
      }
    }

    previous_time = current_time;
    SDL_GetCurrentTime(&current_time);
    float dt = (float)(current_time - previous_time);
    // Nanoseconds -> Milliseconds
    dt /= 1000000.0f;
    printf("fps: %f, Frame Time: %f\n", 1.0 / (dt / 1000.0f), dt);

    // a += 0.03f;
    transform = glms_rotate(glms_mat4_identity(), a, axis);

    float zero = 0.0f;
    image_clear(&pipeline.z_buffer, &zero);

    SDL_LockSurface(pipeline.framebuffer);

    SDL_ClearSurface(pipeline.framebuffer, 0.0, 0.0, 0.0, 1.0);

    pipeline_draw(&pipeline, &model);

    SDL_UnlockSurface(pipeline.framebuffer);

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    BURNRAST_SDL_CHECK(
        SDL_BlitSurface(pipeline.framebuffer, 0, window_surface, 0))
    BURNRAST_SDL_CHECK(SDL_UpdateWindowSurface(window));

    frames++;
  }

  destroy_rasterization_pipeline(&pipeline);
  model_free(&model);
  SDL_Quit();
  return 0;
}

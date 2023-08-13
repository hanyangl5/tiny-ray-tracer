#include "render.h"

#include "box.h"
#include "bvh.h"
#include "camera.h"
#include "constant_medium.h"
#include "flip_normals.h"
#include "hittablelist.h"
#include "image_texture.h"
#include "material.h"
#include "random.h"
#include "rotate.h"
#include "sphere.h"
#include "texture.h"
#include "translate.h"
#include "xyz_rect.h"
#include <chrono>
#include <thread>
#include <vector>

vec3 color(const ray &r, hittable *world, int depth) {
  hit_record rec;
  if (world->hit(r, 0.1, FLT_MAX, rec)) {
    ray scattered;
    vec3 attenuation;
    vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
    if (depth < 100 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
      return emitted + attenuation * color(scattered, world, depth + 1);
    } else {
      return emitted;
    }
  }
  return vec3(0, 0, 0);
}

void render(int w, int h, int spp, unsigned char *color_buffer, hittable *scene,
            camera *cam) {

  auto t1 = std::chrono::system_clock().now();
  std::vector<std::thread> jobs;
  unsigned int thread_count = std::thread::hardware_concurrency() - 1;
  unsigned int thread_h = h / thread_count + 1;
  for (int i = 0; i < thread_count; i++) {
    std::thread t([&, i]() {
      unsigned char *p = color_buffer + 3 * w * thread_h * i;
      for (int j = h - 1 - i * thread_h; j > h - 1 - (i + 1) * thread_h; j--) {
        // std::cout<<j << i << "\n";
        if (j < 0)
          continue;
        for (int i = 0; i < w; i++) {
          vec3 col(0, 0, 0);
          for (int s = 0; s < spp; s++) {
            float u = float(i + random_double()) / float(w);
            float v = float(j + random_double()) / float(h);
            ray r = cam->get_ray(u, v);
            col += color(r, scene, 0);
          }
          col /= float(spp);
          *p++ = int(255.99 * col[0]); /* R */
          *p++ = int(255.99 * col[1]); /* G */
          *p++ = int(255.99 * col[2]); /* B */
        }
      }
    });
    jobs.push_back(std::move(t));
  }
  for (auto &t : jobs) {
    t.join();
  }
  auto t2 = std::chrono::system_clock().now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);
  std::cout << "spend" << double(duration.count()) << "s\n";
  //      unsigned char *p = color_buffer;
  //    for (int j = h - 1; j >= 0; j--) {
  //      for (int i = 0; i < w; i++) {
  //        vec3 col(0, 0, 0);
  //        for (int s = 0; s < spp; s++) {
  //          float u = float(i + random_double()) / float(w);
  //          float v = float(j + random_double()) / float(h);
  //          ray r = cam->get_ray(u, v);
  //          col += color(r, scene, 0);
  //        }
  //        col /= float(spp);
  //        *p++ = int(255.99 * col[0]); /* R */
  //        *p++ = int(255.99 * col[1]); /* G */
  //        *p++ = int(255.99 * col[2]); /* B */
  //      }

  //   }
}
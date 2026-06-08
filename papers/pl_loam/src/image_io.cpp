#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "pl_loam/pl_loam.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace localization_zoo {
namespace pl_loam {

namespace {

int clampInt(int v, int lo, int hi) {
  return std::max(lo, std::min(hi, v));
}

}  // namespace

CameraModel CameraModel::kittiColor02() {
  CameraModel cam;
  cam.width = 1242;
  cam.height = 375;
  cam.fx = 721.5377;
  cam.fy = 721.5377;
  cam.cx = 609.5593;
  cam.cy = 172.854;
  cam.T_velo_cam.setIdentity();
  cam.T_velo_cam << 7.533745e-03, -9.999714e-01, -6.166020e-04, -4.069766e-03,
      1.480227e-02, 7.280733e-04, -9.998902e-01, -7.631618e-02,
      9.998621e-01, 7.523790e-03, 1.480755e-02, -2.717806e-01, 0.0, 0.0, 0.0,
      1.0;
  return cam;
}

CameraModel CameraModel::kittiColor02HalfRes() {
  CameraModel cam = kittiColor02();
  cam.width /= 2;
  cam.height /= 2;
  cam.fx /= 2.0;
  cam.fy /= 2.0;
  cam.cx = (cam.cx + 0.5) / 2.0 - 0.5;
  cam.cy = (cam.cy + 0.5) / 2.0 - 0.5;
  return cam;
}

bool GrayscaleImage::loadRgbPng(const std::string& path, int target_width,
                                int target_height) {
  width = 0;
  height = 0;
  pixels.clear();

  int src_w = 0;
  int src_h = 0;
  int channels = 0;
  unsigned char* rgb =
      stbi_load(path.c_str(), &src_w, &src_h, &channels, 3);
  if (!rgb || src_w <= 0 || src_h <= 0) {
    if (rgb) stbi_image_free(rgb);
    return false;
  }

  const int out_w = target_width > 0 ? target_width : src_w;
  const int out_h = target_height > 0 ? target_height : src_h;
  width = out_w;
  height = out_h;
  pixels.assign(static_cast<size_t>(out_w * out_h), 0);

  for (int y = 0; y < out_h; ++y) {
    for (int x = 0; x < out_w; ++x) {
      const int sx = clampInt(static_cast<int>(std::lround(
                              static_cast<double>(x) * src_w / out_w)),
                              0, src_w - 1);
      const int sy = clampInt(static_cast<int>(std::lround(
                              static_cast<double>(y) * src_h / out_h)),
                              0, src_h - 1);
      const unsigned char* px = rgb + 3 * (sy * src_w + sx);
      const float gray =
          0.299f * px[0] + 0.587f * px[1] + 0.114f * px[2];
      pixels[static_cast<size_t>(y * out_w + x)] =
          static_cast<uint8_t>(clampInt(static_cast<int>(gray + 0.5f), 0, 255));
    }
  }

  stbi_image_free(rgb);
  return true;
}

}  // namespace pl_loam
}  // namespace localization_zoo

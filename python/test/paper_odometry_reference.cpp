// Native oracle for the binding tests. Inputs are little-endian float64 XYZ
// triples exported by the test from the committed point-cloud fixture.
#include "dilo/dilo_paper.h"
#include "imls_slam/imls_paper_odometry.h"
#include "litamin2/litamin2_odometry.h"
#include "suma/suma_paper.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace localization_zoo;

std::vector<Eigen::Vector3d> readScan(const std::string& path) {
  std::ifstream input(path, std::ios::binary | std::ios::ate);
  const auto bytes = input.tellg();
  if (!input || bytes <= 0 || bytes % (3 * sizeof(double)) != 0)
    throw std::runtime_error("invalid float64 XYZ file: " + path);
  input.seekg(0);
  std::vector<Eigen::Vector3d> points;
  for (double xyz[3]; input.read(reinterpret_cast<char*>(xyz), sizeof(xyz));)
    points.emplace_back(xyz[0], xyz[1], xyz[2]);
  return points;
}

int main(int argc, char** argv) {
  try {
    if (argc < 3) throw std::runtime_error("usage: paper_odometry_reference METHOD SCAN.f64 ...");
    const std::string method = argv[1];
    suma::SuMaPaper suma;
    imls_slam::ImlsPaperOdometry imls;
    dilo::DiloPaperOdometry dilo;
    litamin2::LiTAMIN2Odometry litamin;
    std::cout << std::setprecision(17);
    for (int frame = 2; frame < argc; ++frame) {
      const auto scan = readScan(argv[frame]);
      Eigen::Matrix4d pose;
      size_t state = 0;
      int support = 0;
      if (method == "suma_paper") {
        const auto result = suma.process(scan);
        pose = result.pose; state = result.num_surfels; support = result.inliers;
      } else if (method == "imls_slam_paper") {
        const auto result = imls.process(scan);
        pose = result.pose; state = result.model_points; support = result.samples;
      } else if (method == "dilo_paper") {
        pose = dilo.process(scan);
        state = dilo.windowPoses().size(); support = static_cast<int>(dilo.finalizedPoses().size());
      } else if (method == "litamin2_paper") {
        const auto result = litamin.process(scan);
        pose = result.pose; state = result.map_distributions; support = result.correspondences;
      } else {
        throw std::runtime_error("unknown method: " + method);
      }
      for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col) std::cout << pose(row, col) << ',';
      std::cout << state << ',' << support << '\n';
    }
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

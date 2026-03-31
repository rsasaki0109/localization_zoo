#pragma once

#include <Eigen/Core>

#include <vector>

namespace localization_zoo {
namespace scan_context {

struct ScanContextParams {
  int num_rings = 20;
  int num_sectors = 60;
  double max_radius = 80.0;
  double lidar_height = 2.0;
  int exclude_recent_frames = 30;
  int num_candidates = 10;
  double search_ratio = 0.1;
  double distance_threshold = 0.18;
};

struct LoopCandidate {
  bool valid = false;
  int index = -1;
  double distance = 1.0;
  int yaw_shift = 0;
  double yaw_offset_rad = 0.0;
};

class ScanContextManager {
public:
  explicit ScanContextManager(const ScanContextParams& params = ScanContextParams())
      : params_(params) {}

  Eigen::MatrixXd makeScanContext(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::VectorXd makeRingKey(const Eigen::MatrixXd& descriptor) const;
  Eigen::VectorXd makeSectorKey(const Eigen::MatrixXd& descriptor) const;

  int addScan(const std::vector<Eigen::Vector3d>& points);
  LoopCandidate detectLoop(const std::vector<Eigen::Vector3d>& points) const;
  void clear();

  int numScans() const { return static_cast<int>(contexts_.size()); }

private:
  struct StoredContext {
    Eigen::MatrixXd descriptor;
    Eigen::VectorXd ring_key;
    Eigen::VectorXd sector_key;
  };

  int alignSectorKeys(const Eigen::VectorXd& query,
                      const Eigen::VectorXd& candidate) const;
  double compareDescriptors(const Eigen::MatrixXd& query,
                            const Eigen::VectorXd& query_sector_key,
                            const StoredContext& candidate,
                            int* best_shift) const;
  static Eigen::MatrixXd circularShiftColumns(const Eigen::MatrixXd& matrix,
                                              int shift);

  ScanContextParams params_;
  std::vector<StoredContext> contexts_;
};

}  // namespace scan_context
}  // namespace localization_zoo

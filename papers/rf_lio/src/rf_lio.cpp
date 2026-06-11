#include "rf_lio/rf_lio.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace localization_zoo {
namespace rf_lio {

namespace {

constexpr double kPi = 3.14159265358979323846;

std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& points, const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) out.push_back(R * p + t);
  return out;
}

}  // namespace

RfLioPipeline::RfLioPipeline(const RfLioParams& params)
    : params_(params), backend_(params.backend) {}

void RfLioPipeline::setInitialPose(const Eigen::Matrix4d& pose) {
  backend_.setInitialPose(pose);
  previous_pose_ = pose;
  last_delta_ = Eigen::Matrix4d::Identity();
  has_previous_ = false;
  previous_static_scan_.clear();
}

bool RfLioPipeline::projectToRangeImage(const Eigen::Vector3d& p_sensor,
                                        int width, int height, int* idx,
                                        double* range) const {
  const double r = p_sensor.norm();
  if (r < params_.min_range || r > params_.max_range) return false;
  const double yaw = std::atan2(p_sensor.y(), p_sensor.x());
  const double pitch = std::asin(std::clamp(p_sensor.z() / r, -1.0, 1.0));
  const double fov_up = params_.fov_up_deg * kPi / 180.0;
  const double fov_down = params_.fov_down_deg * kPi / 180.0;
  if (pitch > fov_up || pitch < fov_down) return false;
  int u = static_cast<int>((0.5 * (yaw / kPi + 1.0)) *
                           static_cast<double>(width));
  int v = static_cast<int>(((fov_up - pitch) / (fov_up - fov_down)) *
                           static_cast<double>(height));
  u = std::clamp(u, 0, width - 1);
  v = std::clamp(v, 0, height - 1);
  *idx = v * width + u;
  if (range) *range = r;
  return true;
}

RfLioPipeline::RangeImage RfLioPipeline::buildRangeImage(
    const std::vector<Eigen::Vector3d>& points, int width, int height) const {
  RangeImage image;
  image.width = width;
  image.height = height;
  const int n = width * height;
  image.ranges.assign(n, std::numeric_limits<double>::infinity());
  image.valid.assign(n, false);
  for (const auto& p : points) {
    int idx = 0;
    double r = 0.0;
    if (!projectToRangeImage(p, width, height, &idx, &r)) continue;
    if (r < image.ranges[idx]) {
      image.ranges[idx] = r;
      image.valid[idx] = true;
    }
  }
  return image;
}

int RfLioPipeline::foregroundVotes(
    const Eigen::Vector3d& p_sensor,
    const std::vector<RangeImage>& predicted) const {
  int votes = 0;
  for (const auto& image : predicted) {
    int idx = 0;
    double r = 0.0;
    if (!projectToRangeImage(p_sensor, image.width, image.height, &idx, &r)) {
      continue;
    }
    if (image.valid[idx] && r + params_.foreground_margin < image.ranges[idx]) {
      ++votes;
    }
  }
  return votes;
}

std::vector<Eigen::Vector3d> RfLioPipeline::removalFirstFilter(
    const std::vector<Eigen::Vector3d>& frame, int* removed_points,
    int* candidate_points) const {
  if (removed_points) *removed_points = 0;
  if (candidate_points) *candidate_points = 0;
  if (!has_previous_ || previous_static_scan_.empty()) return frame;

  const Eigen::Matrix4d predicted_pose = previous_pose_ * last_delta_;
  const Eigen::Matrix4d previous_to_current =
      predicted_pose.inverse() * previous_pose_;
  const auto predicted_previous =
      transformPoints(previous_static_scan_, previous_to_current);

  const int fine_w = std::max(16, params_.range_image_width);
  const int fine_h = std::max(8, params_.range_image_height);
  const int mid_w = std::max(16, fine_w / 2);
  const int mid_h = std::max(8, fine_h / 2);
  const int coarse_w = std::max(16, fine_w / 4);
  const int coarse_h = std::max(8, fine_h / 4);

  std::vector<RangeImage> predicted;
  predicted.push_back(buildRangeImage(predicted_previous, fine_w, fine_h));
  predicted.push_back(buildRangeImage(predicted_previous, mid_w, mid_h));
  predicted.push_back(buildRangeImage(predicted_previous, coarse_w, coarse_h));

  struct Candidate {
    size_t index = 0;
    double score = 0.0;
  };
  std::vector<Candidate> candidates;
  candidates.reserve(frame.size() / 8);
  std::vector<bool> remove(frame.size(), false);

  for (size_t i = 0; i < frame.size(); ++i) {
    const int votes = foregroundVotes(frame[i], predicted);
    if (votes < params_.min_foreground_votes) continue;
    int fine_idx = 0;
    double r = 0.0;
    double score = static_cast<double>(votes);
    if (projectToRangeImage(frame[i], fine_w, fine_h, &fine_idx, &r) &&
        predicted.front().valid[fine_idx]) {
      score += std::max(0.0, predicted.front().ranges[fine_idx] - r);
    }
    candidates.push_back({i, score});
  }

  if (candidate_points) *candidate_points = static_cast<int>(candidates.size());
  if (candidates.empty()) return frame;

  const size_t max_remove = std::min(
      candidates.size(),
      static_cast<size_t>(std::max(
          0.0, std::floor(params_.max_removal_fraction *
                          static_cast<double>(frame.size())))));
  const size_t min_keep = static_cast<size_t>(std::max(0, params_.min_keep_points));
  const size_t room_remove = frame.size() > min_keep ? frame.size() - min_keep : 0;
  const size_t allowed_remove = std::min(max_remove, room_remove);
  if (allowed_remove == 0) return frame;

  std::partial_sort(candidates.begin(), candidates.begin() + allowed_remove,
                    candidates.end(),
                    [](const Candidate& a, const Candidate& b) {
                      return a.score > b.score;
                    });
  for (size_t i = 0; i < allowed_remove; ++i) remove[candidates[i].index] = true;

  std::vector<Eigen::Vector3d> out;
  out.reserve(frame.size() - allowed_remove);
  int removed = 0;
  for (size_t i = 0; i < frame.size(); ++i) {
    if (remove[i]) {
      ++removed;
      continue;
    }
    out.push_back(frame[i]);
  }
  if (removed_points) *removed_points = removed;
  return out;
}

RfLioResult RfLioPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame,
    const std::vector<imu_preintegration::ImuSample>& imu) {
  RfLioResult out;
  int removed = 0;
  int candidates = 0;
  const auto filtered = removalFirstFilter(frame, &removed, &candidates);
  const Eigen::Matrix4d pose_before = backend_.pose();
  out.backend = backend_.registerFrame(filtered, imu);
  out.pose = out.backend.pose;
  out.removal_first_points = removed;
  out.kept_points = static_cast<int>(filtered.size());
  out.candidate_points = candidates;

  previous_static_scan_ = filtered;
  previous_pose_ = out.pose;
  last_delta_ = pose_before.inverse() * out.pose;
  has_previous_ = true;
  return out;
}

}  // namespace rf_lio
}  // namespace localization_zoo

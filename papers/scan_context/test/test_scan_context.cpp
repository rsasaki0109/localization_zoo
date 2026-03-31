#include "scan_context/scan_context.h"

#include <gtest/gtest.h>

#include <Eigen/Geometry>

#include <cmath>
#include <vector>

using namespace localization_zoo::scan_context;

namespace {

std::vector<Eigen::Vector3d> makeUrbanLikeScan() {
  std::vector<Eigen::Vector3d> points;

  for (int deg = 0; deg < 360; deg += 4) {
    const double rad = deg * M_PI / 180.0;
    const double radius =
        18.0 + 4.0 * std::sin(2.0 * rad) + 2.0 * std::cos(5.0 * rad);
    points.emplace_back(radius * std::cos(rad), radius * std::sin(rad), 0.0);
    points.emplace_back((radius + 1.0) * std::cos(rad),
                        (radius + 1.0) * std::sin(rad),
                        1.0 + 0.5 * std::sin(3.0 * rad));
  }

  const std::vector<Eigen::Vector3d> pillars = {
      {12.0, 3.0, 5.5},
      {-6.0, 18.0, 7.0},
      {-15.0, -8.0, 4.8},
      {8.0, -17.0, 6.2},
  };
  for (const auto& center : pillars) {
    for (int i = 0; i < 30; ++i) {
      const double z = 0.2 * i;
      points.emplace_back(center.x(), center.y(), z);
    }
  }

  return points;
}

std::vector<Eigen::Vector3d> makeDifferentScan() {
  std::vector<Eigen::Vector3d> points;

  for (int deg = 0; deg < 180; deg += 3) {
    const double rad = deg * M_PI / 180.0;
    const double radius = 10.0 + 6.0 * std::sin(4.0 * rad);
    points.emplace_back(radius * std::cos(rad), radius * std::sin(rad), 0.0);
    points.emplace_back((radius + 0.5) * std::cos(rad),
                        (radius + 0.5) * std::sin(rad),
                        2.5 + 0.8 * std::cos(rad));
  }

  for (int i = 0; i < 60; ++i) {
    points.emplace_back(-22.0 + 0.1 * i, -5.0, 3.0 + 0.02 * i);
    points.emplace_back(20.0, 8.0 - 0.12 * i, 1.0 + 0.03 * i);
  }
  return points;
}

std::vector<Eigen::Vector3d> rotateYaw(
    const std::vector<Eigen::Vector3d>& points, double yaw_rad) {
  const Eigen::Matrix3d rotation =
      Eigen::AngleAxisd(yaw_rad, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  std::vector<Eigen::Vector3d> rotated;
  rotated.reserve(points.size());
  for (const auto& point : points) {
    rotated.push_back(rotation * point);
  }
  return rotated;
}

int cyclicShiftError(int lhs, int rhs, int modulus) {
  const int wrapped = (lhs - rhs + modulus) % modulus;
  return std::min(wrapped, modulus - wrapped);
}

}  // namespace

TEST(ScanContext, BuildsDescriptorAndKeys) {
  ScanContextParams params;
  params.num_rings = 10;
  params.num_sectors = 20;
  ScanContextManager manager(params);

  const auto scan = makeUrbanLikeScan();
  const Eigen::MatrixXd descriptor = manager.makeScanContext(scan);
  const Eigen::VectorXd ring_key = manager.makeRingKey(descriptor);
  const Eigen::VectorXd sector_key = manager.makeSectorKey(descriptor);

  EXPECT_EQ(descriptor.rows(), 10);
  EXPECT_EQ(descriptor.cols(), 20);
  EXPECT_EQ(ring_key.size(), 10);
  EXPECT_EQ(sector_key.size(), 20);
  EXPECT_GT(descriptor.maxCoeff(), 0.0);
}

TEST(ScanContext, DetectsYawRotatedLoopCandidate) {
  ScanContextParams params;
  params.exclude_recent_frames = 0;
  params.num_candidates = 3;
  params.distance_threshold = 0.20;
  ScanContextManager manager(params);

  const auto base_scan = makeUrbanLikeScan();
  const auto distractor_scan = makeDifferentScan();
  manager.addScan(base_scan);
  manager.addScan(distractor_scan);

  const double yaw_deg = 30.0;
  const auto rotated_query = rotateYaw(base_scan, yaw_deg * M_PI / 180.0);
  const LoopCandidate candidate = manager.detectLoop(rotated_query);

  ASSERT_TRUE(candidate.valid);
  EXPECT_EQ(candidate.index, 0);
  EXPECT_LT(candidate.distance, 0.20);

  const int expected_shift =
      static_cast<int>(std::round(yaw_deg / 360.0 * params.num_sectors)) %
      params.num_sectors;
  const int reverse_shift =
      (params.num_sectors - expected_shift) % params.num_sectors;
  EXPECT_LE(std::min(cyclicShiftError(candidate.yaw_shift, expected_shift,
                                      params.num_sectors),
                     cyclicShiftError(candidate.yaw_shift, reverse_shift,
                                      params.num_sectors)),
            1);
}

TEST(ScanContext, RejectsDifferentPlace) {
  ScanContextParams params;
  params.exclude_recent_frames = 0;
  params.distance_threshold = 0.15;
  ScanContextManager manager(params);

  manager.addScan(makeUrbanLikeScan());
  const LoopCandidate candidate = manager.detectLoop(makeDifferentScan());

  EXPECT_FALSE(candidate.valid);
}

TEST(ScanContext, ClearsStoredContexts) {
  ScanContextManager manager;
  manager.addScan(makeUrbanLikeScan());
  manager.addScan(makeDifferentScan());
  ASSERT_EQ(manager.numScans(), 2);

  manager.clear();
  EXPECT_EQ(manager.numScans(), 0);
  EXPECT_FALSE(manager.detectLoop(makeUrbanLikeScan()).valid);
}

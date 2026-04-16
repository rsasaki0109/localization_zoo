#include "common/io.h"
#include "test_thresholds.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

using namespace localization_zoo;
using namespace localization_zoo::test;

namespace {

std::string makeTempFilePath() {
  char pattern[] = "/tmp/lz_common_io_test_XXXXXX";
  const int fd = mkstemp(pattern);
  if (fd < 0) {
    throw std::runtime_error("mkstemp failed");
  }
  ::close(fd);
  return std::string(pattern);
}

void writeTextFile(const std::string& path, const std::string& content) {
  std::ofstream out(path);
  if (!out) {
    throw std::runtime_error("failed to write temp file: " + path);
  }
  out << content;
}

TEST(CommonIO, LoadPosesCsvParsesQuaternionOrder) {
  const std::string path = makeTempFilePath();
  const std::string body =
      "timestamp,x,y,z,qx,qy,qz,qw\n"
      "1.25,1,2,3,0,0,0,1\n";
  writeTextFile(path, body);

  const auto poses = io::loadPosesCsv(path);
  ASSERT_EQ(poses.size(), 1u);
  EXPECT_DOUBLE_EQ(poses[0].first, kGoldenPoseTimestamp);
  EXPECT_TRUE(poses[0].second.position.isApprox(
      Eigen::Vector3d(kGoldenPoseX, kGoldenPoseY, kGoldenPoseZ), kEpsilonVector3));
  EXPECT_TRUE(poses[0].second.orientation.isApprox(
      Eigen::Quaterniond(kGoldenQuatW, kGoldenQuatX, kGoldenQuatY, kGoldenQuatZ),
      kEpsilonQuaternion));

  std::remove(path.c_str());
}

TEST(CommonIO, LoadImuCsvParsesGyroAndAccel) {
  const std::string path = makeTempFilePath();
  const std::string body =
      "t,gx,gy,gz,ax,ay,az\n"
      "0.5,0.1,0.2,0.3,1,2,3\n";
  writeTextFile(path, body);

  const auto imu = io::loadImuCsv(path);
  ASSERT_EQ(imu.size(), 1u);
  EXPECT_DOUBLE_EQ(imu[0].timestamp, kGoldenImuTimestamp);
  EXPECT_TRUE(imu[0].angular_velocity.isApprox(
      Eigen::Vector3d(kGyroX, kGyroY, kGyroZ), kEpsilonVector3));
  EXPECT_TRUE(imu[0].linear_acceleration.isApprox(
      Eigen::Vector3d(kAccelX, kAccelY, kAccelZ), kEpsilonVector3));

  std::remove(path.c_str());
}

TEST(CommonIO, LoadPosesCsvMissingFileThrows) {
  EXPECT_THROW(io::loadPosesCsv("/nonexistent/localization_zoo_pose.csv"),
               std::runtime_error);
}

TEST(CommonIO, LoadImuCsvMissingFileThrows) {
  EXPECT_THROW(io::loadImuCsv("/nonexistent/localization_zoo_imu.csv"),
               std::runtime_error);
}

}  // namespace

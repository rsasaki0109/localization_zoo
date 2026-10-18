#include "common/range_filter.h"

#include <pcl/point_types.h>
#include <gtest/gtest.h>

#include <cmath>
#include <limits>

namespace {
using localization_zoo::preprocessing::filterMinRange;

TEST(RangeFilter, DisabledLeavesOrganizedCloudAndMetadataUntouched) {
  pcl::PointCloud<pcl::PointXYZI> cloud;
  cloud.resize(4);
  cloud.width = 2;
  cloud.height = 2;
  cloud.is_dense = false;
  cloud.header.frame_id = "sensor";
  cloud.sensor_origin_ = Eigen::Vector4f(1, 2, 3, 0);
  cloud[0].intensity = 42;
  filterMinRange(cloud);
  EXPECT_EQ(cloud.size(), 4u);
  EXPECT_EQ(cloud.width, 2u);
  EXPECT_EQ(cloud.height, 2u);
  EXPECT_FALSE(cloud.is_dense);
  EXPECT_EQ(cloud.header.frame_id, "sensor");
  EXPECT_EQ(cloud.sensor_origin_, Eigen::Vector4f(1, 2, 3, 0));
  EXPECT_FLOAT_EQ(cloud[0].intensity, 42);
}

TEST(RangeFilter, StrictSphericalBoundaryPreservesOrderAndIntensity) {
  pcl::PointCloud<pcl::PointXYZI> cloud;
  for (const auto& xyz : {Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(3, 4, 0),
                          Eigen::Vector3f(0, 0, -6), Eigen::Vector3f(1, 2, 2)}) {
    pcl::PointXYZI point;
    point.getVector3fMap() = xyz;
    point.intensity = static_cast<float>(cloud.size() + 10);
    cloud.push_back(point);
  }
  cloud.is_dense = false;
  cloud.header.frame_id = "sensor";
  filterMinRange(cloud, 5);
  ASSERT_EQ(cloud.size(), 2u);
  EXPECT_FLOAT_EQ(cloud[0].x, 3);
  EXPECT_FLOAT_EQ(cloud[0].y, 4);
  EXPECT_FLOAT_EQ(cloud[0].intensity, 11);
  EXPECT_FLOAT_EQ(cloud[1].z, -6);
  EXPECT_FLOAT_EQ(cloud[1].intensity, 12);
  EXPECT_EQ(cloud.width, 2u);
  EXPECT_EQ(cloud.height, 1u);
  EXPECT_FALSE(cloud.is_dense);
  EXPECT_EQ(cloud.header.frame_id, "sensor");
}

TEST(RangeFilter, XyzPathUsesFloatThresholdAndKeepsNonFinitePolicy) {
  pcl::PointCloud<pcl::PointXYZ> cloud;
  cloud.push_back(pcl::PointXYZ(std::nextafter(5.0f, 0.0f), 0, 0));
  cloud.push_back(pcl::PointXYZ(5, 0, 0));
  cloud.push_back(pcl::PointXYZ(std::nextafter(5.0f, 6.0f), 0, 0));
  cloud.push_back(pcl::PointXYZ(std::numeric_limits<float>::quiet_NaN(), 0, 0));
  cloud.push_back(pcl::PointXYZ(std::numeric_limits<float>::infinity(), 0, 0));
  filterMinRange(cloud, 5);
  ASSERT_EQ(cloud.size(), 4u);
  EXPECT_FLOAT_EQ(cloud[0].x, 5);
  EXPECT_GT(cloud[1].x, 5);
  EXPECT_TRUE(std::isnan(cloud[2].x));
  EXPECT_TRUE(std::isinf(cloud[3].x));
}

TEST(RangeFilter, EmptyAndAllRemovedCloudsHaveOneRowWhenEnabled) {
  pcl::PointCloud<pcl::PointXYZ> cloud;
  filterMinRange(cloud, 5);
  EXPECT_EQ(cloud.width, 0u);
  EXPECT_EQ(cloud.height, 1u);
  cloud.push_back(pcl::PointXYZ(1, 2, 3));
  filterMinRange(cloud, 5);
  EXPECT_TRUE(cloud.empty());
  EXPECT_EQ(cloud.width, 0u);
  EXPECT_EQ(cloud.height, 1u);
}

TEST(RangeFilter, NonPositiveAndNanRangesKeepHistoricalNoOp) {
  for (double range : {0.0, -1.0, std::numeric_limits<double>::quiet_NaN()}) {
    pcl::PointCloud<pcl::PointXYZ> cloud;
    cloud.push_back(pcl::PointXYZ(0, 0, 0));
    filterMinRange(cloud, range);
    ASSERT_EQ(cloud.size(), 1u);
  }
}
}  // namespace

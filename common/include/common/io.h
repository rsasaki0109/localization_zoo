#pragma once

#include "common/types.h"

#include <string>
#include <vector>

namespace localization_zoo {
namespace io {

/// PCD ファイルから点群を読み込み
PointCloudPtr loadPointCloud(const std::string& path);

/// CSV から Pose 列を読み込み (timestamp, x, y, z, qx, qy, qz, qw)
std::vector<std::pair<double, Pose>> loadPosesCsv(const std::string& path);

/// CSV から IMU データを読み込み
std::vector<ImuData> loadImuCsv(const std::string& path);

}  // namespace io
}  // namespace localization_zoo

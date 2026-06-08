#pragma once

#include <Eigen/Core>

#include <string>
#include <vector>

namespace localization_zoo {
namespace nn_zupt {

struct Conv1dLayer {
  int in_channels = 0;
  int out_channels = 0;
  int kernel_size = 1;
  std::vector<double> weight;
  std::vector<double> bias;
};

struct DenseLayer {
  int in_features = 0;
  int out_features = 0;
  std::vector<double> weight;
  std::vector<double> bias;
};

struct ZuptCnnModel {
  int window_size = 50;
  int channels = 6;
  std::vector<Conv1dLayer> convs;
  std::vector<int> pool_kernel;
  std::vector<DenseLayer> fcs;
};

struct ZuptForwardResult {
  double stop_prob = 0.0;
};

bool loadZuptCnnJson(const std::string& path, ZuptCnnModel* model,
                     std::string* error = nullptr);

ZuptForwardResult forwardZuptCnn(const ZuptCnnModel& model,
                                 const std::vector<double>& window);

}  // namespace nn_zupt
}  // namespace localization_zoo

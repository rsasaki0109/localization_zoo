#pragma once

#include <Eigen/Core>

#include <string>
#include <vector>

namespace localization_zoo {
namespace odonet {

/// Paper-inspired 1D CNN speed regressor (OdoNet, arXiv:2109.03091).
/// Weights are exported from ``papers/odonet/scripts/train_odonet.py``.
struct Conv1dLayer {
  int in_channels = 0;
  int out_channels = 0;
  int kernel_size = 1;
  std::vector<double> weight;  // out * in * kernel
  std::vector<double> bias;    // out
};

struct DenseLayer {
  int in_features = 0;
  int out_features = 0;
  std::vector<double> weight;  // out * in (row-major)
  std::vector<double> bias;
};

struct Cnn1dModel {
  int window_size = 50;
  int channels = 6;
  double speed_scale = 30.0;
  std::vector<Conv1dLayer> convs;
  std::vector<int> pool_kernel;  // one per conv
  std::vector<DenseLayer> fcs;
};

bool loadCnn1dModelJson(const std::string& path, Cnn1dModel* model,
                        std::string* error = nullptr);

/// Input ``window`` is row-major [time][channel], length = window_size * channels.
double forwardSpeed(const Cnn1dModel& model,
                    const std::vector<double>& window);

}  // namespace odonet
}  // namespace localization_zoo

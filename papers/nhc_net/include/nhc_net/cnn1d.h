#pragma once

#include <Eigen/Core>

#include <string>
#include <vector>

namespace localization_zoo {
namespace nhc_net {

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

struct VmscCnnModel {
  int window_size = 50;
  int channels = 6;
  int num_classes = 4;
  double nhc_scale = 2.0;
  std::vector<Conv1dLayer> convs;
  std::vector<int> pool_kernel;
  std::vector<DenseLayer> shared_fcs;
  DenseLayer class_head;
  DenseLayer nhc_head;
};

struct VmscForwardResult {
  int motion_class = 0;
  std::vector<double> class_probs;
  double lateral_vel = 0.0;
  double vertical_vel = 0.0;
};

bool loadVmscCnnJson(const std::string& path, VmscCnnModel* model,
                     std::string* error = nullptr);

VmscForwardResult forwardVmsc(const VmscCnnModel& model,
                              const std::vector<double>& window);

}  // namespace nhc_net
}  // namespace localization_zoo

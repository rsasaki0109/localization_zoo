#include "nhc_net/cnn1d.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

namespace localization_zoo {
namespace nhc_net {
namespace {

double relu(double x) { return x > 0.0 ? x : 0.0; }

std::vector<double> conv1d(const std::vector<double>& in, int in_ch, int length,
                           const Conv1dLayer& layer) {
  const int out_ch = layer.out_channels;
  const int k = layer.kernel_size;
  const int pad = k / 2;
  const int out_len = length;
  std::vector<double> out(static_cast<size_t>(out_ch * out_len), 0.0);
  for (int oc = 0; oc < out_ch; ++oc) {
    for (int t = 0; t < out_len; ++t) {
      double sum = layer.bias[static_cast<size_t>(oc)];
      for (int ic = 0; ic < in_ch; ++ic) {
        for (int ki = 0; ki < k; ++ki) {
          const int ti = t + ki - pad;
          if (ti < 0 || ti >= length) continue;
          const size_t w_idx = static_cast<size_t>((oc * in_ch + ic) * k + ki);
          const size_t in_idx = static_cast<size_t>(ic * length + ti);
          sum += layer.weight[w_idx] * in[in_idx];
        }
      }
      out[static_cast<size_t>(oc * out_len + t)] = sum;
    }
  }
  return out;
}

std::vector<double> maxPool1d(const std::vector<double>& in, int channels,
                              int length, int kernel) {
  const int out_len = length / kernel;
  std::vector<double> out(static_cast<size_t>(channels * out_len), 0.0);
  for (int c = 0; c < channels; ++c) {
    for (int t = 0; t < out_len; ++t) {
      double m = -1e30;
      for (int k = 0; k < kernel; ++k) {
        m = std::max(m, in[static_cast<size_t>(c * length + t * kernel + k)]);
      }
      out[static_cast<size_t>(c * out_len + t)] = m;
    }
  }
  return out;
}

std::vector<double> dense(const std::vector<double>& in, const DenseLayer& layer) {
  std::vector<double> out(static_cast<size_t>(layer.out_features), 0.0);
  for (int o = 0; o < layer.out_features; ++o) {
    double sum = layer.bias[static_cast<size_t>(o)];
    for (int i = 0; i < layer.in_features; ++i) {
      sum += layer.weight[static_cast<size_t>(o * layer.in_features + i)] *
             in[static_cast<size_t>(i)];
    }
    out[static_cast<size_t>(o)] = sum;
  }
  return out;
}

bool parseDoubleArray(const std::string& key, const std::string& json,
                      std::vector<double>* out) {
  const std::string needle = "\"" + key + "\": [";
  const size_t pos = json.find(needle);
  if (pos == std::string::npos) return false;
  size_t i = pos + needle.size();
  out->clear();
  while (i < json.size()) {
    while (i < json.size() && (json[i] == ' ' || json[i] == '\n')) ++i;
    if (i < json.size() && json[i] == ']') return true;
    size_t j = i;
    while (j < json.size() && json[j] != ',' && json[j] != ']') ++j;
    out->push_back(std::stod(json.substr(i, j - i)));
    i = j;
    if (i < json.size() && json[i] == ',') ++i;
  }
  return false;
}

int parseIntField(const std::string& key, const std::string& json, int fallback) {
  const std::string needle = "\"" + key + "\": ";
  const size_t pos = json.find(needle);
  if (pos == std::string::npos) return fallback;
  return std::stoi(json.substr(pos + needle.size()));
}

double parseDoubleField(const std::string& key, const std::string& json,
                        double fallback) {
  const std::string needle = "\"" + key + "\": ";
  const size_t pos = json.find(needle);
  if (pos == std::string::npos) return fallback;
  return std::stod(json.substr(pos + needle.size()));
}

std::vector<std::string> splitObjects(const std::string& json,
                                      const std::string& array_key) {
  const std::string needle = "\"" + array_key + "\": [";
  const size_t start = json.find(needle);
  if (start == std::string::npos) return {};
  size_t i = start + needle.size();
  std::vector<std::string> blocks;
  int depth = 0;
  size_t block_start = std::string::npos;
  for (; i < json.size(); ++i) {
    if (json[i] == '{') {
      if (depth == 0) block_start = i;
      ++depth;
    } else if (json[i] == '}') {
      --depth;
      if (depth == 0 && block_start != std::string::npos) {
        blocks.push_back(json.substr(block_start, i - block_start + 1));
        block_start = std::string::npos;
      }
    } else if (json[i] == ']' && depth == 0) {
      break;
    }
  }
  return blocks;
}

bool parseConvBlock(const std::string& block, Conv1dLayer* layer) {
  layer->in_channels = parseIntField("in_channels", block, 0);
  layer->out_channels = parseIntField("out_channels", block, 0);
  layer->kernel_size = parseIntField("kernel_size", block, 1);
  if (!parseDoubleArray("weight", block, &layer->weight)) return false;
  if (!parseDoubleArray("bias", block, &layer->bias)) return false;
  return layer->in_channels > 0 && layer->out_channels > 0;
}

bool parseDenseBlock(const std::string& block, DenseLayer* layer) {
  layer->in_features = parseIntField("in_features", block, 0);
  layer->out_features = parseIntField("out_features", block, 0);
  if (!parseDoubleArray("weight", block, &layer->weight)) return false;
  if (!parseDoubleArray("bias", block, &layer->bias)) return false;
  return layer->in_features > 0 && layer->out_features > 0;
}

std::string extractObject(const std::string& json, const std::string& key) {
  const std::string needle = "\"" + key + "\": {";
  const size_t pos = json.find(needle);
  if (pos == std::string::npos) return {};
  size_t i = pos + needle.size() - 1;
  int depth = 0;
  size_t start = std::string::npos;
  for (; i < json.size(); ++i) {
    if (json[i] == '{') {
      if (depth == 0) start = i;
      ++depth;
    } else if (json[i] == '}') {
      --depth;
      if (depth == 0 && start != std::string::npos) {
        return json.substr(start, i - start + 1);
      }
    }
  }
  return {};
}

std::vector<double> softmax(const std::vector<double>& logits) {
  double max_logit = logits.empty() ? 0.0 : *std::max_element(logits.begin(), logits.end());
  std::vector<double> exps(logits.size());
  double sum = 0.0;
  for (size_t i = 0; i < logits.size(); ++i) {
    exps[i] = std::exp(logits[i] - max_logit);
    sum += exps[i];
  }
  if (sum <= 0.0) return exps;
  for (double& v : exps) v /= sum;
  return exps;
}

}  // namespace

bool loadVmscCnnJson(const std::string& path, VmscCnnModel* model,
                     std::string* error) {
  std::ifstream in(path);
  if (!in) {
    if (error) *error = "cannot open " + path;
    return false;
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  const std::string json = ss.str();

  model->window_size = parseIntField("window_size", json, 50);
  model->channels = parseIntField("channels", json, 6);
  model->num_classes = parseIntField("num_classes", json, 4);
  model->nhc_scale = parseDoubleField("nhc_scale", json, 2.0);
  model->convs.clear();
  model->pool_kernel.clear();
  model->shared_fcs.clear();

  for (const auto& block : splitObjects(json, "convs")) {
    Conv1dLayer layer;
    if (!parseConvBlock(block, &layer)) {
      if (error) *error = "invalid conv block";
      return false;
    }
    model->convs.push_back(layer);
    model->pool_kernel.push_back(parseIntField("pool", block, 2));
  }
  for (const auto& block : splitObjects(json, "shared_fcs")) {
    DenseLayer layer;
    if (!parseDenseBlock(block, &layer)) {
      if (error) *error = "invalid shared fc";
      return false;
    }
    model->shared_fcs.push_back(layer);
  }
  if (!parseDenseBlock(extractObject(json, "class_head"), &model->class_head) ||
      !parseDenseBlock(extractObject(json, "nhc_head"), &model->nhc_head)) {
    if (error) *error = "missing heads";
    return false;
  }
  return !model->convs.empty();
}

VmscForwardResult forwardVmsc(const VmscCnnModel& model,
                              const std::vector<double>& window) {
  VmscForwardResult out;
  if (static_cast<int>(window.size()) != model.window_size * model.channels) {
    return out;
  }

  int channels = model.channels;
  int length = model.window_size;
  std::vector<double> activ = window;
  for (size_t li = 0; li < model.convs.size(); ++li) {
    activ = conv1d(activ, channels, length, model.convs[li]);
    for (double& v : activ) v = relu(v);
    const int pool = model.pool_kernel[li];
    activ = maxPool1d(activ, model.convs[li].out_channels, length, pool);
    channels = model.convs[li].out_channels;
    length /= pool;
  }

  std::vector<double> feat = activ;
  for (const auto& fc : model.shared_fcs) {
    feat = dense(feat, fc);
    for (double& v : feat) v = relu(v);
  }

  const std::vector<double> class_logits = dense(feat, model.class_head);
  out.class_probs = softmax(class_logits);
  out.motion_class = static_cast<int>(
      std::max_element(out.class_probs.begin(), out.class_probs.end()) -
      out.class_probs.begin());

  const std::vector<double> nhc = dense(feat, model.nhc_head);
  out.lateral_vel = nhc[0] * model.nhc_scale;
  out.vertical_vel = nhc.size() > 1 ? nhc[1] * model.nhc_scale : 0.0;
  return out;
}

}  // namespace nhc_net
}  // namespace localization_zoo

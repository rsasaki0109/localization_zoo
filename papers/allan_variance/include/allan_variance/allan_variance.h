#pragma once

// Allan variance analysis of static inertial sensor data.
//
// El-Sheimy, Hou, Niu, "Analysis and modeling of inertial sensors using Allan
// variance", IEEE Trans. Instrum. Meas. 57(1), 2008, and IEEE Std 952-1997.
// The noise-parameter fit follows the protocol of the TUM VI benchmark
// (Schubert et al., IROS 2018, Sec. III-C and Appendix A).

#include <optional>
#include <vector>

namespace localization_zoo {
namespace allan_variance {

struct AllanPoint {
  double tau = 0.0;   // averaging time [s]
  double adev = 0.0;  // Allan deviation [signal unit]
  long clusters = 0;  // number of difference terms behind the estimate
};

/// Cluster sizes m (samples per average) spaced logarithmically from 1 to
/// floor((num_samples - 1) / 2), points_per_decade per factor of ten, unique.
std::vector<long> logClusterSizes(long num_samples, int points_per_decade);

/// Overlapping Allan deviation (TUM VI eq. 11): for averages g_i over m
/// samples, sigma^2(m tau0) = sum_i (g_{i+m} - g_i)^2 / (2 (M - 2m + 1)).
/// Computed in O(M) per cluster size from a cumulative sum.
std::vector<AllanPoint> overlappingAllanDeviation(const std::vector<double>& samples,
                                                  double sampling_rate,
                                                  const std::vector<long>& cluster_sizes);

/// Line of fixed log-log slope through the points with tau_min <= tau <=
/// tau_max (least squares in log10, i.e. the mean offset), evaluated at
/// tau_eval. Returns nullopt if no point lies in the range.
std::optional<double> fixedSlopeFit(const std::vector<AllanPoint>& curve, double slope,
                                    double tau_min, double tau_max, double tau_eval);

/// Line of fixed slope through the single point whose local log-log slope is
/// closest to `slope` (the usual automatic choice, e.g. MATLAB's allanvar
/// example), evaluated at tau_eval.
std::optional<double> tangentFit(const std::vector<AllanPoint>& curve, double slope,
                                 double tau_eval);

struct NoiseParameters {
  // Continuous-time white noise density sigma_w: line of slope -1/2 at tau = 1 s
  // [unit / sqrt(Hz)], e.g. rad/s/sqrt(Hz) for a gyroscope.
  double white_noise_density = 0.0;
  // Bias random walk sigma_b: line of slope +1/2 at tau = 3 s
  // [unit / s / sqrt(Hz)].
  double bias_random_walk = 0.0;
  // Bias instability: minimum of the Allan deviation / sqrt(2 ln 2 / pi).
  double bias_instability = 0.0;
  double bias_instability_tau = 0.0;
};

/// Automatic extraction (tangent fits and the flicker floor).
NoiseParameters extractNoiseParameters(const std::vector<AllanPoint>& curve);

/// Element-wise arithmetic mean of curves evaluated on the same cluster sizes.
std::vector<AllanPoint> meanCurve(const std::vector<std::vector<AllanPoint>>& curves);

}  // namespace allan_variance
}  // namespace localization_zoo

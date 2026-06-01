#pragma once

// Named tolerances and reference values for common/ tests.
// Change thresholds here so pass criteria stay explicit and in one place.

namespace localization_zoo::test {

// --- floating-point tolerances (what "clears" the bar) ---
inline constexpr double kEpsilonSkew = 1e-12;
inline constexpr double kEpsilonAngleBound = 1e-12;
inline constexpr double kEpsilonAngleEquiv = 1e-9;
inline constexpr double kEpsilonRotation = 1e-12;
inline constexpr double kEpsilonOrthonormal = 1e-10;
inline constexpr double kEpsilonQuaternion = 1e-12;
inline constexpr double kEpsilonVector3 = 1e-12;

// --- CSV / parser golden values (fixed expected numbers) ---
inline constexpr double kGoldenPoseTimestamp = 1.25;
inline constexpr double kGoldenPoseX = 1.0;
inline constexpr double kGoldenPoseY = 2.0;
inline constexpr double kGoldenPoseZ = 3.0;
inline constexpr double kGoldenQuatW = 1.0;
inline constexpr double kGoldenQuatX = 0.0;
inline constexpr double kGoldenQuatY = 0.0;
inline constexpr double kGoldenQuatZ = 0.0;

inline constexpr double kGoldenImuTimestamp = 0.5;
inline constexpr double kGyroX = 0.1;
inline constexpr double kGyroY = 0.2;
inline constexpr double kGyroZ = 0.3;
inline constexpr double kAccelX = 1.0;
inline constexpr double kAccelY = 2.0;
inline constexpr double kAccelZ = 3.0;

}  // namespace localization_zoo::test

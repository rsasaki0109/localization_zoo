#pragma once

#include <utility>

namespace localization_zoo {
namespace registration {

// Optional coarse initialization before the ordinary registration. Indices
// count registrations, starting at 1 after the map-initialization scan (0).
// This is the existing IMLS-SLAM opt-in startup policy, not a paper default.
// Callbacks capture the same pose: coarse updates it, then fine refines it.
// Correspondences, residuals, stopping rules and acceptance remain local to
// each method. The fine stage always runs and supplies the return value.
template <class Coarse, class Fine>
decltype(auto) initialCoarseThenFine(int registration_index, int coarse_frames,
                                   int coarse_iterations, Coarse&& coarse,
                                   Fine&& fine, bool* coarse_ran = nullptr) {
  if (coarse_ran != nullptr) *coarse_ran = false;
  if (registration_index > 0 && registration_index <= coarse_frames && coarse_iterations > 0) {
    std::forward<Coarse>(coarse)(coarse_iterations);
    if (coarse_ran != nullptr) *coarse_ran = true;
  }
  return std::forward<Fine>(fine)();
}

}  // namespace registration
}  // namespace localization_zoo

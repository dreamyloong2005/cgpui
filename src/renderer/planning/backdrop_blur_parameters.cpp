#include "cgpui/renderer/planning/backdrop_blur_parameters.hpp"

namespace cgpui {

std::uint32_t backdrop_blur_downsample_factor(float sigma) {
  if (!(sigma > 4.0F)) return 1;
  if (sigma <= 16.0F) return 2;
  if (sigma <= 48.0F) return 4;
  return 8;
}

} // namespace cgpui

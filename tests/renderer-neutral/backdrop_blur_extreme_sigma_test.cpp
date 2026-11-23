#include "cgpui/renderer/planning/backdrop_blur_plan.hpp"
#include "cgpui/renderer/planning/backdrop_blur_parameters.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>

int main() {
  using namespace cgpui;
  constexpr PixelExtent extent{64, 48};
  BackdropBlurDraw draw{.rect = {{12, 8}, {20, 16}}};
  unsigned failures = 0;
  // 1e12 keeps sigma^2 finite but its kernel radius is not representable
  // as int. Larger finite inputs also overflow the old float square.
  for (const float blur : {1.0e12F, 1.0e20F, std::numeric_limits<float>::max()}) {
    draw.blur_radius = blur;
    const auto plan = plan_backdrop_blur(std::array{draw}, extent);
    if (plan.passes.size() != 1 || plan.vertices.size() != 6) {
      std::cerr << "finite extreme blur must retain its visible pass\n";
      ++failures;
      continue;
    }
    const auto& pass = plan.passes[0];
    if (!std::isfinite(pass.sigma) || pass.sigma < 0 || pass.sigma > blur ||
        pass.radius < 0 || pass.radius > kBackdropBlurMaxRadius || pass.radius != 48) {
      std::cerr << std::setprecision(std::numeric_limits<float>::max_digits10)
                << "blur=" << blur << " produced sigma=" << pass.sigma
                << " radius=" << pass.radius << "; expected finite sigma and radius 48\n";
      ++failures;
    }
    if (pass.region.x < 0 || pass.region.y < 0 || pass.region.width == 0 ||
        pass.region.height == 0 ||
        std::uint64_t(pass.region.x) + pass.region.width > extent.width ||
        std::uint64_t(pass.region.y) + pass.region.height > extent.height ||
        pass.region.x != 0 || pass.region.y != 0 ||
        pass.region.width != 64 || pass.region.height != 48 ||
        pass.factor != 8 || pass.texels.width != 8 || pass.texels.height != 6) {
      std::cerr << "extreme blur region must remain clamped to the finite frame\n";
      ++failures;
    }
    for (const auto& vertex : plan.vertices) {
      for (const float value : vertex.position) {
        if (!std::isfinite(value)) { ++failures; break; }
      }
      for (const float value : vertex.region) {
        if (!std::isfinite(value)) { ++failures; break; }
      }
    }
  }
  for (const float blur : {std::numeric_limits<float>::quiet_NaN(),
                           std::numeric_limits<float>::infinity(),
                           -std::numeric_limits<float>::infinity()}) {
    draw.blur_radius = blur;
    const auto plan = plan_backdrop_blur(std::array{draw}, extent);
    if (plan.passes.size() != 1 || plan.passes[0].sigma != 0 ||
        plan.passes[0].radius != 0 || plan.passes[0].factor != 1) {
      std::cerr << "nonfinite authored blur must retain its existing zero-blur fallback\n";
      ++failures;
    }
  }
  return failures == 0 ? 0 : 1;
}

#include "cgpui/renderer/planning/backdrop_blur_plan.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <limits>

int main() {
  using namespace cgpui;
  constexpr auto signed_max = static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max());
  constexpr auto unsigned_max = std::numeric_limits<std::uint32_t>::max();
  const BackdropBlurDraw small{.rect = {{1, 1}, {8, 8}}};
  unsigned failures = 0;
  // Extents outside signed scissor-coordinate representation are not a
  // supported CPU planning domain, regardless of native hardware limits.
  for (const PixelExtent extent : {PixelExtent{unsigned_max, 32}, {32, unsigned_max},
                                   {signed_max + 1, 32}, {32, signed_max + 1}}) {
    const auto plan = plan_backdrop_blur(std::array{small}, extent);
    if (!plan.passes.empty() || !plan.vertices.empty()) {
      std::cerr << "unrepresentable extent accepted: " << extent.width << 'x' << extent.height << '\n';
      ++failures;
    }
  }
  // 2^24+1 rounds down in float. A clamped region must retain the exact
  // integer frame edge, rather than silently dropping its final column.
  const BackdropBlurDraw wide{.rect = {{-10, 0}, {2000000000.0F, 32}}};
  const auto odd = plan_backdrop_blur(std::array{wide}, {16777217, 23});
  if (odd.passes.size() != 1 || odd.passes[0].region.x != 0 || odd.passes[0].region.y != 0 ||
      odd.passes[0].region.width != 16777217 || odd.passes[0].region.height != 23 ||
      odd.passes[0].texels.width != 16777217 || odd.passes[0].texels.height != 23) {
    std::cerr << "large odd extent lost its exact frame boundary\n";
    ++failures;
  }
  BackdropBlurDraw huge_blur = small;
  huge_blur.blur_radius = std::numeric_limits<float>::max();
  const auto full = plan_backdrop_blur(std::array{huge_blur}, {signed_max, signed_max});
  if (full.passes.size() != 1 || full.passes[0].region.x != 0 || full.passes[0].region.y != 0 ||
      full.passes[0].region.width != signed_max || full.passes[0].region.height != signed_max ||
      full.passes[0].factor != 8 || full.passes[0].texels.width != 268435456 ||
      full.passes[0].texels.height != 268435456) {
    std::cerr << "INT32_MAX extent rounded outside the frame or overflowed ceil division\n";
    ++failures;
  }
  // This origin is exactly representable as float. Its right edge is past
  // INT32_MAX, so only the last 127 columns of the valid frame remain.
  const BackdropBlurDraw edge{.rect = {{2147483520.0F, 1}, {256, 10}}};
  const auto clipped = plan_backdrop_blur(std::array{edge}, {signed_max, 32});
  if (clipped.passes.size() != 1 || clipped.passes[0].region.x != 2147483520 ||
      clipped.passes[0].region.y != 0 || clipped.passes[0].region.width != 127 ||
      clipped.passes[0].region.height != 12) {
    std::cerr << "right-edge scissor conversion is not representable and frame-clamped\n";
    ++failures;
  }
  return failures == 0 ? 0 : 1;
}

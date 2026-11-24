#include "../../src/renderer/vulkan/vulkan_glyph_atlas_validation_internal.hpp"

#include <limits>

int main() {
  using namespace cgpui;
  const auto small = vulkan_glyph_atlas_extent({255.5F, 256.0F}, 256);
  if (!small || small->width != 256 || small->height != 256) return 1;
  if (vulkan_glyph_atlas_extent({256.1F, 256.0F}, 256) ||
      vulkan_glyph_atlas_extent({256.0F, 256.0F}, 128) ||
      vulkan_glyph_atlas_extent({1.0F, 1.0F}, 0)) return 2;
  for (const float dimension : {0.0F, -1.0F, std::numeric_limits<float>::infinity(),
                                std::numeric_limits<float>::quiet_NaN()}) {
    if (vulkan_glyph_atlas_extent({dimension, 1.0F}, 256) ||
        vulkan_glyph_atlas_extent({1.0F, dimension}, 256)) return 3;
  }
  // This float is 2^32, not UINT32_MAX: reject before the narrowing conversion.
  if (vulkan_glyph_atlas_extent({static_cast<float>(std::numeric_limits<std::uint32_t>::max()), 1.0F},
          std::numeric_limits<std::uint32_t>::max())) return 4;
  return 0;
}

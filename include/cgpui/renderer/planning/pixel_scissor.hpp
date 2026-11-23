#pragma once

#include <cstdint>

namespace cgpui {

struct PixelExtent {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
};

struct PixelScissor {
  std::int32_t x = 0;
  std::int32_t y = 0;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
};

} // namespace cgpui

#pragma once
#include "cgpui/core/geometry.hpp"
#include <optional>

namespace cgpui {
// Resolved, validated unavailability. Not a pending request or native texture.
struct Dx12MissingImageDraw {
  std::optional<Rect> clip_rect;
};
} // namespace cgpui

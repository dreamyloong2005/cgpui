#pragma once

#include "cgpui/renderer/renderer_commands.hpp"

namespace cgpui {
// Device-scaled, owned request; resolved in its original command slot at Present.
struct Dx12PendingImageDraw {
  ImageDraw image;
};
} // namespace cgpui

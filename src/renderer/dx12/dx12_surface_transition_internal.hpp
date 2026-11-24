#pragma once

#include "cgpui/core/error.hpp"

namespace cgpui {
struct Dx12State;
Result<void> dx12_replace_surface_alpha(Dx12State&, bool premultiplied);
} // namespace cgpui

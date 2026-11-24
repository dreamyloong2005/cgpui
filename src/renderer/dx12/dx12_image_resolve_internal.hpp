#pragma once

#include "dx12_draw_commands_internal.hpp"

namespace cgpui {
Result<Dx12TexturedDraw> dx12_resolve_image_draw(
    Dx12State& state, const Dx12PendingImageDraw& request);
} // namespace cgpui

#pragma once

#include "dx12_draw_commands_internal.hpp"

namespace cgpui {
using Dx12ResolvedImageDraw = std::variant<Dx12TexturedDraw, Dx12MissingImageDraw>;
Result<Dx12ResolvedImageDraw> dx12_resolve_image_draw(
    Dx12State& state, const Dx12PendingImageDraw& request);
} // namespace cgpui

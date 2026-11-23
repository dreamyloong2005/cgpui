#pragma once

#include "dx12_draw_commands_internal.hpp"

namespace cgpui {
struct Dx12State;

Result<std::vector<Dx12TexturedDraw>> dx12_build_text_draws(
    Dx12State& state, const TextDraw& device_text);

} // namespace cgpui

#pragma once
#include "dx12_draw_commands_internal.hpp"
namespace cgpui {
Result<void> dx12_record_frame_draws(Dx12State&, Color, std::span<const Dx12DrawCommand>, bool capture, bool mutations);
} // namespace cgpui

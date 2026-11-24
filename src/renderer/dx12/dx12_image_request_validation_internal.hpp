#pragma once
#include "dx12_draw_commands_internal.hpp"

namespace cgpui {
Result<void> dx12_validate_image_request_identities(std::span<const Dx12DrawCommand> draws);
} // namespace cgpui

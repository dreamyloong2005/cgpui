#pragma once

#include "cgpui/renderer/image_decode.hpp"

namespace cgpui::detail {

[[nodiscard]] EncodedImageFormat detect_encoded_image_format(
    std::span<const std::uint8_t> bytes);

} // namespace cgpui::detail

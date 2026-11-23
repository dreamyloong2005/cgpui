#pragma once

#include <cstdint>

namespace cgpui {

inline constexpr std::uint32_t kBackdropBlurVerticesPerPass = 6;
inline constexpr int kBackdropBlurMaxRadius = 48;

// 1 up to sigma 4 device px, 2 up to 16, 4 up to 48, else 8.
[[nodiscard]] std::uint32_t backdrop_blur_downsample_factor(float sigma);

} // namespace cgpui

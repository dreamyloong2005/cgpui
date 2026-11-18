#pragma once

#include <array>
#include <cstdint>

namespace cgpui {

enum class FontStyle : std::uint8_t {
  normal,
  italic,
};

// An OpenType feature toggle, e.g. {"liga", 0}. `value` 0 disables the
// feature, 1 enables it; alternates may use larger values.
struct FontFeature {
  std::array<char, 4> tag{};
  std::uint32_t value = 1;

  friend bool operator==(const FontFeature&, const FontFeature&) = default;
};

[[nodiscard]] constexpr FontFeature font_feature(
    const char (&tag)[5],
    std::uint32_t value) {
  return FontFeature{
      .tag = {tag[0], tag[1], tag[2], tag[3]},
      .value = value,
  };
}

inline constexpr std::uint16_t font_weight_normal = 400;
inline constexpr std::uint16_t font_weight_bold = 700;

} // namespace cgpui

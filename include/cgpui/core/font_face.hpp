#pragma once

#include "cgpui/core/font.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace cgpui {

enum class FontSource {
  platform,
  test,
};

struct FontUnicodeRange {
  char32_t first = U'\0';
  char32_t last = U'\0';

  friend bool operator==(const FontUnicodeRange&, const FontUnicodeRange&) =
      default;
};

struct FontFaceDescriptor {
  FontDescriptor font;
  std::string postscript_name;
  FontSource source = FontSource::platform;
  std::string path;
  std::vector<FontUnicodeRange> coverage;
  // Native face identity: index inside a font collection file and
  // synthesized styling applied by the platform.
  std::uint32_t face_index = 0;
  bool synthetic_bold = false;
  bool synthetic_oblique = false;
  bool monospaced = false;

  friend bool operator==(
      const FontFaceDescriptor&,
      const FontFaceDescriptor&) = default;
};

} // namespace cgpui

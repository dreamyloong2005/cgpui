#pragma once

#include "text_shaping_native_internal.hpp"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace cgpui {

// One fontconfig candidate in fallback order (FcFontSort), primary first.
struct LinuxFontFace {
  FontFaceDescriptor face;
  // Opaque FcCharSet*, referenced for the lifetime of the cached set.
  std::shared_ptr<void> charset;
};

// Sorted, coverage-trimmed fontconfig set for a descriptor (cached).
// Empty when fontconfig has no fonts at all.
[[nodiscard]] const std::vector<LinuxFontFace>& fontconfig_fallback_faces(
    const FontDescriptor& font);

[[nodiscard]] bool linux_face_covers(const LinuxFontFace& face, char32_t codepoint);

// FreeType metrics at `font_size` logical px and the space glyph for a face.
[[nodiscard]] bool linux_native_face(
    const FontFaceDescriptor& face,
    float font_size,
    NativeShapedFace& output);

// FreeType faces are not thread-safe: hold this mutex while using the
// FT_Face (as void*) returned by linux_ft_face_locked.
[[nodiscard]] std::mutex& linux_ft_mutex();
[[nodiscard]] void* linux_ft_face_locked(const std::string& path, std::uint32_t index);

} // namespace cgpui

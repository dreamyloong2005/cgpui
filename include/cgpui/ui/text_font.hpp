#pragma once

#include "cgpui/ui/style.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

enum class FontSource {
  platform,
  test,
};

struct FontFaceDescriptor {
  FontDescriptor font;
  std::string postscript_name;
  FontSource source = FontSource::platform;
  std::string path;

  friend bool operator==(
      const FontFaceDescriptor&,
      const FontFaceDescriptor&) = default;
};

class FontFallbackChain {
 public:
  [[nodiscard]] bool empty() const;
  [[nodiscard]] std::size_t size() const;
  [[nodiscard]] std::span<const FontFaceDescriptor* const> faces() const;
  [[nodiscard]] const FontFaceDescriptor* primary() const;

  void add_face(const FontFaceDescriptor* face);

 private:
  [[nodiscard]] bool contains(const FontFaceDescriptor* face) const;

  std::vector<const FontFaceDescriptor*> faces_;
};

class FontDatabase {
 public:
  [[nodiscard]] bool empty() const;
  [[nodiscard]] std::size_t face_count() const;
  [[nodiscard]] std::span<const FontFaceDescriptor> faces() const;

  void add_face(FontFaceDescriptor face);
  void add_generic_fallback_family(std::string_view family);

  [[nodiscard]] std::span<const std::string> generic_fallback_families() const;
  [[nodiscard]] const FontFaceDescriptor* resolve(
      const FontDescriptor& descriptor) const;
  [[nodiscard]] FontFallbackChain resolve_chain(
      const FontDescriptor& descriptor) const;

 private:
  [[nodiscard]] const FontFaceDescriptor* find_face_for_family(
      std::string_view family) const;

  std::vector<FontFaceDescriptor> faces_;
  std::vector<std::string> generic_fallback_families_;
};

[[nodiscard]] FontDatabase font_database_from_discovered_faces(
    std::span<const FontFaceDescriptor> faces);

[[nodiscard]] FontDatabase discover_test_fonts(
    std::span<const FontFaceDescriptor> faces);

} // namespace cgpui

#pragma once

#include "cgpui/ui/style.hpp"

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <utility>
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
  [[nodiscard]] bool empty() const {
    return faces_.empty();
  }

  [[nodiscard]] std::size_t size() const {
    return faces_.size();
  }

  [[nodiscard]] std::span<const FontFaceDescriptor* const> faces() const {
    return faces_;
  }

  [[nodiscard]] const FontFaceDescriptor* primary() const {
    return faces_.empty() ? nullptr : faces_.front();
  }

  void add_face(const FontFaceDescriptor* face) {
    if (face == nullptr || contains(face)) {
      return;
    }
    faces_.push_back(face);
  }

 private:
  [[nodiscard]] bool contains(const FontFaceDescriptor* face) const {
    return std::find(faces_.begin(), faces_.end(), face) != faces_.end();
  }

  std::vector<const FontFaceDescriptor*> faces_;
};

class FontDatabase {
 public:
  [[nodiscard]] bool empty() const {
    return faces_.empty();
  }

  [[nodiscard]] std::size_t face_count() const {
    return faces_.size();
  }

  [[nodiscard]] std::span<const FontFaceDescriptor> faces() const {
    return faces_;
  }

  void add_face(FontFaceDescriptor face) {
    if (face.font.family.empty()) {
      return;
    }
    for (const auto& existing : faces_) {
      if (existing == face) {
        return;
      }
    }
    faces_.push_back(std::move(face));
  }

  void add_generic_fallback_family(std::string_view family) {
    if (family.empty()) {
      return;
    }
    if (std::find(
            generic_fallback_families_.begin(),
            generic_fallback_families_.end(),
            family) != generic_fallback_families_.end()) {
      return;
    }
    generic_fallback_families_.push_back(std::string(family));
  }

  [[nodiscard]] std::span<const std::string> generic_fallback_families()
      const {
    return generic_fallback_families_;
  }

  [[nodiscard]] const FontFaceDescriptor* resolve(
      const FontDescriptor& descriptor) const {
    if (descriptor.family.empty()) {
      return faces_.empty() ? nullptr : &faces_.front();
    }
    return find_face_for_family(descriptor.family);
  }

  [[nodiscard]] FontFallbackChain resolve_chain(
      const FontDescriptor& descriptor) const {
    FontFallbackChain chain;
    if (faces_.empty()) {
      return chain;
    }
    if (descriptor.family.empty()) {
      chain.add_face(&faces_.front());
      return chain;
    }

    chain.add_face(find_face_for_family(descriptor.family));
    for (const auto& fallback_family : generic_fallback_families_) {
      chain.add_face(find_face_for_family(fallback_family));
    }
    chain.add_face(&faces_.front());
    return chain;
  }

 private:
  [[nodiscard]] const FontFaceDescriptor* find_face_for_family(
      std::string_view family) const {
    for (const auto& face : faces_) {
      if (face.font.family == family) {
        return &face;
      }
    }
    return nullptr;
  }

  std::vector<FontFaceDescriptor> faces_;
  std::vector<std::string> generic_fallback_families_;
};

[[nodiscard]] inline FontDatabase font_database_from_discovered_faces(
    std::span<const FontFaceDescriptor> faces) {
  FontDatabase database;
  for (const auto& face : faces) {
    database.add_face(face);
  }
  return database;
}

[[nodiscard]] inline FontDatabase discover_test_fonts(
    std::span<const FontFaceDescriptor> faces) {
  return font_database_from_discovered_faces(faces);
}


} // namespace cgpui

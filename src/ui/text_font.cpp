#include "cgpui/ui/text_font.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace cgpui {

bool FontFallbackChain::empty() const {
  return faces_.empty();
}

std::size_t FontFallbackChain::size() const {
  return faces_.size();
}

std::span<const FontFaceDescriptor* const> FontFallbackChain::faces() const {
  return faces_;
}

const FontFaceDescriptor* FontFallbackChain::primary() const {
  return faces_.empty() ? nullptr : faces_.front();
}

void FontFallbackChain::add_face(const FontFaceDescriptor* face) {
  if (face == nullptr || contains(face)) {
    return;
  }
  faces_.push_back(face);
}

bool FontFallbackChain::contains(const FontFaceDescriptor* face) const {
  return std::find(faces_.begin(), faces_.end(), face) != faces_.end();
}

bool FontDatabase::empty() const {
  return faces_.empty();
}

std::size_t FontDatabase::face_count() const {
  return faces_.size();
}

std::span<const FontFaceDescriptor> FontDatabase::faces() const {
  return faces_;
}

void FontDatabase::add_face(FontFaceDescriptor face) {
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

void FontDatabase::add_generic_fallback_family(std::string_view family) {
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

std::span<const std::string> FontDatabase::generic_fallback_families() const {
  return generic_fallback_families_;
}

const FontFaceDescriptor* FontDatabase::resolve(
    const FontDescriptor& descriptor) const {
  if (descriptor.family.empty()) {
    return faces_.empty() ? nullptr : &faces_.front();
  }
  return find_face_for_family(descriptor.family);
}

FontFallbackChain FontDatabase::resolve_chain(
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

const FontFaceDescriptor* FontDatabase::find_face_for_family(
    std::string_view family) const {
  for (const auto& face : faces_) {
    if (face.font.family == family) {
      return &face;
    }
  }
  return nullptr;
}

FontDatabase font_database_from_discovered_faces(
    std::span<const FontFaceDescriptor> faces) {
  FontDatabase database;
  for (const auto& face : faces) {
    database.add_face(face);
  }
  return database;
}

FontDatabase discover_test_fonts(std::span<const FontFaceDescriptor> faces) {
  return font_database_from_discovered_faces(faces);
}

} // namespace cgpui

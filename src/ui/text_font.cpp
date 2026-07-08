#include "cgpui/ui/text_font.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace cgpui {

bool font_unicode_range_contains(
    const FontUnicodeRange& range,
    char32_t codepoint) {
  return range.first <= range.last && range.first <= codepoint &&
      codepoint <= range.last;
}

bool font_face_declares_coverage(const FontFaceDescriptor& face) {
  return !face.coverage.empty();
}

bool font_face_covers_codepoint(
    const FontFaceDescriptor& face,
    char32_t codepoint) {
  if (!font_face_declares_coverage(face)) {
    return true;
  }
  return std::any_of(
      face.coverage.begin(),
      face.coverage.end(),
      [codepoint](const FontUnicodeRange& range) {
        return font_unicode_range_contains(range, codepoint);
      });
}

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

FontFallbackChain FontDatabase::resolve_chain_for_codepoint(
    const FontDescriptor& descriptor,
    char32_t codepoint) const {
  FontFallbackChain chain;
  if (faces_.empty()) {
    return chain;
  }

  if (!descriptor.family.empty()) {
    chain.add_face(find_covered_face_for_family(
        descriptor.family,
        codepoint));
  }
  for (const auto& fallback_family : generic_fallback_families_) {
    chain.add_face(find_covered_face_for_family(fallback_family, codepoint));
  }
  const FontFaceDescriptor& primary_face = faces_.front();
  if (font_face_declares_coverage(primary_face) &&
      font_face_covers_codepoint(primary_face, codepoint)) {
    chain.add_face(&primary_face);
  }
  return chain.empty() ? resolve_chain(descriptor) : chain;
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

const FontFaceDescriptor* FontDatabase::find_covered_face_for_family(
    std::string_view family,
    char32_t codepoint) const {
  const FontFaceDescriptor* face = find_face_for_family(family);
  if (face == nullptr || !font_face_declares_coverage(*face) ||
      !font_face_covers_codepoint(*face, codepoint)) {
    return nullptr;
  }
  return face;
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

#include "wayland_font_discovery_internal.hpp"

#if defined(CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND)
#include <fontconfig/fontconfig.h>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

struct FcPatternDeleter {
  void operator()(FcPattern* pattern) const {
    if (pattern != nullptr) {
      FcPatternDestroy(pattern);
    }
  }
};

struct FcObjectSetDeleter {
  void operator()(FcObjectSet* object_set) const {
    if (object_set != nullptr) {
      FcObjectSetDestroy(object_set);
    }
  }
};

struct FcFontSetDeleter {
  void operator()(FcFontSet* font_set) const {
    if (font_set != nullptr) {
      FcFontSetDestroy(font_set);
    }
  }
};

std::string fontconfig_string(FcPattern* pattern, const char* object) {
  if (pattern == nullptr || object == nullptr) {
    return {};
  }

  FcChar8* value = nullptr;
  if (FcPatternGetString(pattern, object, 0, &value) != FcResultMatch ||
      value == nullptr) {
    return {};
  }
  return reinterpret_cast<const char*>(value);
}

const char* fontconfig_postscript_object() {
#if defined(FC_POSTSCRIPT_NAME)
  return FC_POSTSCRIPT_NAME;
#else
  return "postscriptname";
#endif
}

FontFaceDescriptor font_face_from_fontconfig_pattern(FcPattern* pattern) {
  std::string family = fontconfig_string(pattern, FC_FAMILY);
  std::string postscript_name =
      fontconfig_string(pattern, fontconfig_postscript_object());
  std::string path = fontconfig_string(pattern, FC_FILE);

  if (postscript_name.empty()) {
    postscript_name = fontconfig_string(pattern, FC_STYLE);
  }
  if (postscript_name.empty()) {
    postscript_name = family;
  }
  if (path.empty()) {
    path = std::string("fontconfig://") + family;
  }

  return FontFaceDescriptor{
      .font = FontDescriptor{.family = std::move(family)},
      .postscript_name = std::move(postscript_name),
      .source = FontSource::platform,
      .path = std::move(path),
  };
}

} // namespace

PlatformFontDiscoveryResult wayland_discover_fonts_with_fontconfig() {
  if (FcInit() == FcFalse) {
    return wayland_deterministic_font_fallback();
  }

  std::unique_ptr<FcPattern, FcPatternDeleter> pattern(FcPatternCreate());
  std::unique_ptr<FcObjectSet, FcObjectSetDeleter> objects(FcObjectSetBuild(
      FC_FAMILY,
      FC_STYLE,
      FC_FILE,
      fontconfig_postscript_object(),
      nullptr));
  if (!pattern || !objects) {
    return wayland_deterministic_font_fallback();
  }

  std::unique_ptr<FcFontSet, FcFontSetDeleter> font_set(
      FcFontList(nullptr, pattern.get(), objects.get()));
  if (!font_set || font_set->nfont <= 0 || font_set->fonts == nullptr) {
    return wayland_deterministic_font_fallback();
  }

  std::vector<FontFaceDescriptor> records;
  records.reserve(static_cast<std::size_t>(font_set->nfont));
  for (int index = 0; index < font_set->nfont; ++index) {
    FcPattern* font = font_set->fonts[index];
    if (font == nullptr || fontconfig_string(font, FC_FAMILY).empty()) {
      continue;
    }
    records.push_back(font_face_from_fontconfig_pattern(font));
  }

  if (records.empty()) {
    return wayland_deterministic_font_fallback();
  }

  return make_platform_font_discovery_result(
      PlatformFontDiscoveryBackend::fontconfig,
      PlatformFontDiscoveryStatus::native_available,
      std::move(records));
}

} // namespace cgpui
#else
namespace cgpui {

PlatformFontDiscoveryResult wayland_discover_fonts_with_fontconfig() {
  return wayland_deterministic_font_fallback();
}

} // namespace cgpui
#endif

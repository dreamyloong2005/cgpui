#include "text_fontconfig_linux_internal.hpp"

#include "cgpui/ui/text_font_defaults.hpp"

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <algorithm>
#include <map>
#include <strings.h>
#include <tuple>
#include <utility>

namespace cgpui {
namespace {

using FaceSetKey = std::tuple<std::string, std::uint16_t, FontStyle>;

std::string pattern_string(FcPattern* pattern, const char* object) {
  FcChar8* value = nullptr;
  return FcPatternGetString(pattern, object, 0, &value) == FcResultMatch && value != nullptr
      ? std::string(reinterpret_cast<const char*>(value))
      : std::string{};
}

int pattern_int(FcPattern* pattern, const char* object, int fallback) {
  int value = fallback;
  return FcPatternGetInteger(pattern, object, 0, &value) == FcResultMatch ? value : fallback;
}

FcPattern* request_pattern(const FontDescriptor& font) {
  FcPattern* pattern = FcPatternCreate();
  const std::string family = resolve_platform_font_family(font.family);
  FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(family.c_str()));
  FcPatternAddInteger(pattern, FC_WEIGHT, FcWeightFromOpenType(font.weight));
  FcPatternAddInteger(pattern, FC_SLANT,
                      font.style == FontStyle::italic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);
  FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);
  return pattern;
}

LinuxFontFace candidate_from(FcPattern* rendered, const FontDescriptor& request) {
  FcBool embolden = FcFalse;
  FcPatternGetBool(rendered, FC_EMBOLDEN, 0, &embolden);
  FcCharSet* charset = nullptr;
  FcPatternGetCharSet(rendered, FC_CHARSET, 0, &charset);
  const int slant = pattern_int(rendered, FC_SLANT, FC_SLANT_ROMAN);
  LinuxFontFace candidate{
      .face = FontFaceDescriptor{
          .font = FontDescriptor{
              .family = pattern_string(rendered, FC_FAMILY),
              .weight = static_cast<std::uint16_t>(FcWeightToOpenType(
                  pattern_int(rendered, FC_WEIGHT, FC_WEIGHT_REGULAR))),
              .style = slant == FC_SLANT_ROMAN ? FontStyle::normal : FontStyle::italic,
          },
          .postscript_name = pattern_string(rendered, FC_POSTSCRIPT_NAME),
          .source = FontSource::platform,
          .path = pattern_string(rendered, FC_FILE),
          .face_index = static_cast<std::uint32_t>(pattern_int(rendered, FC_INDEX, 0)),
          .synthetic_bold = embolden == FcTrue,
          .synthetic_oblique =
              request.style == FontStyle::italic && slant == FC_SLANT_ROMAN,
          .monospaced = pattern_int(rendered, FC_SPACING, FC_PROPORTIONAL) >= FC_MONO,
      },
  };
  if (charset != nullptr) {
    candidate.charset = std::shared_ptr<void>(
        FcCharSetCopy(charset), [](void* value) {
          FcCharSetDestroy(static_cast<FcCharSet*>(value));
        });
  }
  return candidate;
}

std::vector<LinuxFontFace> sorted_faces(const FontDescriptor& font) {
  std::vector<LinuxFontFace> faces;
  if (FcInit() == FcFalse) return faces;
  FcPattern* pattern = request_pattern(font);
  FcResult result = FcResultNoMatch;
  FcFontSet* set = FcFontSort(nullptr, pattern, FcTrue, nullptr, &result);
  for (int index = 0; set != nullptr && index < set->nfont && faces.size() < 64U; ++index) {
    FcPattern* rendered = FcFontRenderPrepare(nullptr, pattern, set->fonts[index]);
    if (rendered == nullptr) continue;
    LinuxFontFace candidate = candidate_from(rendered, font);
    FcPatternDestroy(rendered);
    const bool duplicate = std::ranges::any_of(faces, [&](const LinuxFontFace& face) {
      return face.face.path == candidate.face.path &&
          face.face.face_index == candidate.face.face_index;
    });
    if (!candidate.face.path.empty() && !duplicate) faces.push_back(std::move(candidate));
  }
  if (set != nullptr) FcFontSetDestroy(set);
  FcPatternDestroy(pattern);
  return faces;
}

FT_Library freetype_library() {
  static FT_Library library = [] {
    FT_Library created = nullptr;
    return FT_Init_FreeType(&created) == 0 ? created : nullptr;
  }();
  return library;
}

float unscaled_bearing_y(FT_Face face, char32_t codepoint) {
  const FT_UInt glyph = FT_Get_Char_Index(face, codepoint);
  if (glyph == 0 || FT_Load_Glyph(face, glyph, FT_LOAD_NO_SCALE) != 0) return 0.0F;
  return static_cast<float>(face->glyph->metrics.horiBearingY);
}

} // namespace

const std::vector<LinuxFontFace>& fontconfig_fallback_faces(const FontDescriptor& font) {
  static std::mutex mutex;
  static std::map<FaceSetKey, std::vector<LinuxFontFace>> sets;
  const std::lock_guard lock(mutex);
  FaceSetKey key{font.family, font.weight, font.style};
  if (const auto found = sets.find(key); found != sets.end()) return found->second;
  return sets.emplace(std::move(key), sorted_faces(font)).first->second;
}

bool linux_face_covers(const LinuxFontFace& face, char32_t codepoint) {
  return face.charset != nullptr &&
      FcCharSetHasChar(static_cast<FcCharSet*>(face.charset.get()), codepoint) == FcTrue;
}

std::mutex& linux_ft_mutex() {
  static std::mutex mutex;
  return mutex;
}

void* linux_ft_face_locked(const std::string& path, std::uint32_t index) {
  static std::map<std::pair<std::string, std::uint32_t>, FT_Face> faces;
  auto key = std::make_pair(path, index);
  if (const auto found = faces.find(key); found != faces.end()) return found->second;
  FT_Face face = nullptr;
  FT_Library library = freetype_library();
  if (library == nullptr ||
      FT_New_Face(library, path.c_str(), static_cast<FT_Long>(index), &face) != 0) {
    return nullptr;
  }
  faces.emplace(std::move(key), face);
  return face;
}

bool linux_native_face(
    const FontFaceDescriptor& descriptor,
    float font_size,
    NativeShapedFace& output) {
  const std::lock_guard lock(linux_ft_mutex());
  auto* face = static_cast<FT_Face>(linux_ft_face_locked(descriptor.path, descriptor.face_index));
  if (face == nullptr || face->units_per_EM == 0) return false;
  const float unit = font_size / static_cast<float>(face->units_per_EM);
  const float ascent = static_cast<float>(face->ascender);
  const float descent = -static_cast<float>(face->descender);
  const auto* os2 = static_cast<const TT_OS2*>(FT_Get_Sfnt_Table(face, FT_SFNT_OS2));
  float cap = os2 != nullptr && os2->version >= 2 ? os2->sCapHeight : 0.0F;
  float x_height = os2 != nullptr && os2->version >= 2 ? os2->sxHeight : 0.0F;
  if (cap <= 0.0F) cap = unscaled_bearing_y(face, U'H');
  if (x_height <= 0.0F) x_height = unscaled_bearing_y(face, U'x');
  output.face = descriptor;
  output.metrics = TextFontMetrics{
      .ascent = ascent * unit,
      .descent = descent * unit,
      .line_gap = std::max(0.0F, static_cast<float>(face->height) - ascent - descent) * unit,
      .cap_height = cap * unit,
      .x_height = x_height * unit,
      .underline_position = -static_cast<float>(face->underline_position) * unit,
      .underline_thickness = static_cast<float>(face->underline_thickness) * unit,
  };
  const FT_UInt space = FT_Get_Char_Index(face, U' ');
  output.space_glyph_id = space;
  if (space != 0 && FT_Load_Glyph(face, space, FT_LOAD_NO_SCALE) == 0) {
    output.space_advance = static_cast<float>(face->glyph->metrics.horiAdvance) * unit;
  }
  return true;
}

bool native_font_family_installed(std::string_view family) {
  const std::vector<LinuxFontFace>& faces =
      fontconfig_fallback_faces(FontDescriptor{.family = std::string(family)});
  return !faces.empty() &&
      strcasecmp(faces.front().face.font.family.c_str(), std::string(family).c_str()) == 0;
}

} // namespace cgpui

#include "text_dwrite_win32_internal.hpp"

#include <algorithm>
#include <map>
#include <mutex>
#include <tuple>
#include <utility>

namespace cgpui {
namespace {

std::string narrow(std::wstring_view text) {
  if (text.empty()) return {};
  const int length = WideCharToMultiByte(CP_UTF8, 0, text.data(),
      static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
  std::string output(static_cast<std::size_t>(std::max(0, length)), '\0');
  WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
      output.data(), length, nullptr, nullptr);
  return output;
}

std::string family_name_of(IDWriteFont* font) {
  ComPtr<IDWriteFontFamily> family;
  ComPtr<IDWriteLocalizedStrings> names;
  if (FAILED(font->GetFontFamily(family.GetAddressOf())) ||
      FAILED(family->GetFamilyNames(names.GetAddressOf()))) {
    return {};
  }
  UINT32 index = 0;
  BOOL exists = FALSE;
  if (FAILED(names->FindLocaleName(L"en-us", &index, &exists)) || !exists) {
    index = 0;
  }
  UINT32 length = 0;
  if (FAILED(names->GetStringLength(index, &length))) return {};
  std::wstring name(length + 1U, L'\0');
  if (FAILED(names->GetString(index, name.data(), length + 1U))) return {};
  name.resize(length);
  return narrow(name);
}

std::optional<std::string> local_file_path(IDWriteFontFace* face) {
  UINT32 count = 1;
  ComPtr<IDWriteFontFile> file;
  if (FAILED(face->GetFiles(&count, file.GetAddressOf())) || !file) {
    return std::nullopt;
  }
  const void* key = nullptr;
  UINT32 key_size = 0;
  ComPtr<IDWriteFontFileLoader> loader;
  ComPtr<IDWriteLocalFontFileLoader> local;
  if (FAILED(file->GetReferenceKey(&key, &key_size)) ||
      FAILED(file->GetLoader(loader.GetAddressOf())) ||
      FAILED(loader.As(&local))) {
    return std::nullopt;
  }
  UINT32 length = 0;
  if (FAILED(local->GetFilePathLengthFromKey(key, key_size, &length))) {
    return std::nullopt;
  }
  std::wstring path(length + 1U, L'\0');
  if (FAILED(local->GetFilePathFromKey(key, key_size, path.data(), length + 1U))) {
    return std::nullopt;
  }
  path.resize(length);
  return narrow(path);
}

TextFontMetrics metrics_of(IDWriteFontFace* face, float font_size) {
  DWRITE_FONT_METRICS design{};
  face->GetMetrics(&design);
  const float unit = design.designUnitsPerEm == 0
      ? 0.0F
      : font_size / static_cast<float>(design.designUnitsPerEm);
  return TextFontMetrics{
      .ascent = design.ascent * unit,
      .descent = design.descent * unit,
      .line_gap = design.lineGap * unit,
      .cap_height = design.capHeight * unit,
      .x_height = design.xHeight * unit,
      .underline_position = -design.underlinePosition * unit,
      .underline_thickness = design.underlineThickness * unit,
  };
}

} // namespace

IDWriteFactory2* dwrite_factory() {
  static const ComPtr<IDWriteFactory2> factory = [] {
    ComPtr<IDWriteFactory2> created;
    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory2),
            reinterpret_cast<IUnknown**>(created.GetAddressOf())))) {
      created.Reset();
    }
    return created;
  }();
  return factory.Get();
}

std::wstring dwrite_wide(std::string_view text) {
  if (text.empty()) return {};
  const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(),
      static_cast<int>(text.size()), nullptr, 0);
  std::wstring output(static_cast<std::size_t>(std::max(0, length)), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
      output.data(), length);
  return output;
}

std::optional<DWriteUtf16Text> dwrite_utf16_from_utf8(std::string_view text) {
  DWriteUtf16Text result;
  result.text.reserve(text.size());
  result.utf8_offsets.reserve(text.size() + 1U);
  std::size_t offset = 0;
  while (offset < text.size()) {
    std::size_t length = 0;
    const char32_t codepoint = text_shaping_decode_utf8(text, offset, &length);
    const bool valid = codepoint <= 0x10FFFFU &&
        !(0xD800U <= codepoint && codepoint <= 0xDFFFU) &&
        (length == 1U) == (codepoint < 0x80U);
    if (!valid) return std::nullopt;
    if (codepoint >= 0x10000U) {
      const char32_t value = codepoint - 0x10000U;
      result.text.push_back(static_cast<wchar_t>(0xD800U + (value >> 10U)));
      result.text.push_back(static_cast<wchar_t>(0xDC00U + (value & 0x3FFU)));
      result.utf8_offsets.push_back(offset);
      result.utf8_offsets.push_back(offset);
    } else {
      result.text.push_back(static_cast<wchar_t>(codepoint));
      result.utf8_offsets.push_back(offset);
    }
    offset += length;
  }
  result.utf8_offsets.push_back(text.size());
  return result;
}

ComPtr<IDWriteFont> dwrite_find_font(
    std::string_view family,
    std::uint16_t weight,
    FontStyle style) {
  IDWriteFactory2* factory = dwrite_factory();
  ComPtr<IDWriteFontCollection> collection;
  if (factory == nullptr ||
      FAILED(factory->GetSystemFontCollection(collection.GetAddressOf()))) {
    return {};
  }
  const std::wstring name = dwrite_wide(family);
  UINT32 index = 0;
  BOOL exists = FALSE;
  ComPtr<IDWriteFontFamily> font_family;
  ComPtr<IDWriteFont> font;
  if (name.empty() ||
      FAILED(collection->FindFamilyName(name.c_str(), &index, &exists)) ||
      !exists || FAILED(collection->GetFontFamily(index, font_family.GetAddressOf())) ||
      FAILED(font_family->GetFirstMatchingFont(
          static_cast<DWRITE_FONT_WEIGHT>(std::clamp<int>(weight, 1, 999)),
          DWRITE_FONT_STRETCH_NORMAL,
          style == FontStyle::italic ? DWRITE_FONT_STYLE_ITALIC
                                     : DWRITE_FONT_STYLE_NORMAL,
          font.GetAddressOf()))) {
    return {};
  }
  return font;
}

bool native_font_family_installed(std::string_view family) {
  return static_cast<bool>(dwrite_find_font(family, font_weight_normal,
                                            FontStyle::normal));
}

std::optional<DWriteFace> dwrite_face_for_font(IDWriteFont* font, float font_size) {
  DWriteFace result;
  if (font == nullptr || FAILED(font->CreateFontFace(result.face.GetAddressOf()))) {
    return std::nullopt;
  }
  std::optional<std::string> path = local_file_path(result.face.Get());
  if (!path) return std::nullopt;
  const DWRITE_FONT_SIMULATIONS simulations = result.face->GetSimulations();
  ComPtr<IDWriteFont1> font1;
  const bool monospaced = SUCCEEDED(font->QueryInterface(IID_PPV_ARGS(&font1))) &&
      font1->IsMonospacedFont() != FALSE;
  result.info.face = FontFaceDescriptor{
      .font = FontDescriptor{
          .family = family_name_of(font),
          .weight = static_cast<std::uint16_t>(font->GetWeight()),
          .style = font->GetStyle() == DWRITE_FONT_STYLE_NORMAL
              ? FontStyle::normal : FontStyle::italic,
      },
      .source = FontSource::platform,
      .path = std::move(*path),
      .face_index = result.face->GetIndex(),
      .synthetic_bold = (simulations & DWRITE_FONT_SIMULATIONS_BOLD) != 0,
      .synthetic_oblique = (simulations & DWRITE_FONT_SIMULATIONS_OBLIQUE) != 0,
      .monospaced = monospaced,
  };
  result.info.metrics = metrics_of(result.face.Get(), font_size);
  const UINT32 space = U' ';
  UINT16 space_glyph = 0;
  DWRITE_GLYPH_METRICS glyph_metrics{};
  DWRITE_FONT_METRICS design{};
  result.face->GetMetrics(&design);
  if (SUCCEEDED(result.face->GetGlyphIndices(&space, 1, &space_glyph)) &&
      SUCCEEDED(result.face->GetDesignGlyphMetrics(&space_glyph, 1, &glyph_metrics)) &&
      design.designUnitsPerEm != 0) {
    result.info.space_glyph_id = space_glyph;
    result.info.space_advance = glyph_metrics.advanceWidth * font_size /
        static_cast<float>(design.designUnitsPerEm);
  }
  return result;
}

ComPtr<IDWriteFontFace> dwrite_face_from_identity(
    const std::string& path,
    std::uint32_t face_index,
    bool synthetic_bold,
    bool synthetic_oblique) {
  using Key = std::tuple<std::string, std::uint32_t, bool, bool>;
  static std::mutex mutex;
  static std::map<Key, ComPtr<IDWriteFontFace>> faces;
  const std::lock_guard lock(mutex);
  Key key{path, face_index, synthetic_bold, synthetic_oblique};
  if (const auto found = faces.find(key); found != faces.end()) {
    return found->second;
  }
  IDWriteFactory2* factory = dwrite_factory();
  ComPtr<IDWriteFontFile> file;
  BOOL supported = FALSE;
  DWRITE_FONT_FILE_TYPE file_type{};
  DWRITE_FONT_FACE_TYPE face_type{};
  UINT32 face_count = 0;
  ComPtr<IDWriteFontFace> face;
  const std::wstring wide_path = dwrite_wide(path);
  const auto simulations = static_cast<DWRITE_FONT_SIMULATIONS>(
      (synthetic_bold ? DWRITE_FONT_SIMULATIONS_BOLD : 0) |
      (synthetic_oblique ? DWRITE_FONT_SIMULATIONS_OBLIQUE : 0));
  if (factory == nullptr ||
      FAILED(factory->CreateFontFileReference(wide_path.c_str(), nullptr,
                                              file.GetAddressOf())) ||
      FAILED(file->Analyze(&supported, &file_type, &face_type, &face_count)) ||
      !supported || face_index >= face_count ||
      FAILED(factory->CreateFontFace(face_type, 1, file.GetAddressOf(),
                                     face_index, simulations, face.GetAddressOf()))) {
    return {};
  }
  faces.emplace(std::move(key), face);
  return face;
}

} // namespace cgpui

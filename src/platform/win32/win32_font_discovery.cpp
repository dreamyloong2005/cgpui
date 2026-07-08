#include "win32_internal.hpp"

#include <dwrite.h>
#include <wrl/client.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

using Microsoft::WRL::ComPtr;

std::string localized_string_at(IDWriteLocalizedStrings* strings, UINT32 index) {
  if (strings == nullptr) {
    return {};
  }

  UINT32 length = 0;
  if (FAILED(strings->GetStringLength(index, &length))) {
    return {};
  }
  std::vector<wchar_t> value(length + 1, L'\0');
  if (FAILED(strings->GetString(index, value.data(), length + 1))) {
    return {};
  }
  return utf8_from_utf16(std::wstring_view(value.data(), length));
}

std::string preferred_localized_string(IDWriteLocalizedStrings* strings) {
  if (strings == nullptr || strings->GetCount() == 0) {
    return {};
  }

  UINT32 index = 0;
  BOOL exists = FALSE;
  if (FAILED(strings->FindLocaleName(L"en-us", &index, &exists)) || !exists) {
    index = 0;
    exists = FALSE;
    (void)strings->FindLocaleName(L"en", &index, &exists);
    if (!exists) {
      index = 0;
    }
  }
  return localized_string_at(strings, index);
}

std::string directwrite_postscript_name(
    IDWriteFont* font,
    std::string_view fallback_family) {
  if (font == nullptr) {
    return std::string(fallback_family);
  }

  BOOL exists = FALSE;
  ComPtr<IDWriteLocalizedStrings> names;
  if (FAILED(font->GetInformationalStrings(
          DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_NAME,
          &names,
          &exists)) ||
      !exists || !names) {
    return std::string(fallback_family);
  }

  const std::string postscript_name = preferred_localized_string(names.Get());
  return postscript_name.empty() ? std::string(fallback_family)
                                 : postscript_name;
}

PlatformFontDiscoveryResult win32_deterministic_font_fallback() {
  std::vector<FontFaceDescriptor> records{
      FontFaceDescriptor{
          .font = FontDescriptor{.family = "Segoe UI"},
          .postscript_name = "SegoeUI",
          .source = FontSource::platform,
          .path = "win32://Segoe UI",
      },
  };
  return make_platform_font_discovery_result(
      PlatformFontDiscoveryBackend::direct_write,
      PlatformFontDiscoveryStatus::deterministic_fallback,
      std::move(records));
}

} // namespace

PlatformFontDiscoveryResult win32_discover_fonts() {
  ComPtr<IDWriteFactory> factory;
  if (FAILED(DWriteCreateFactory(
          DWRITE_FACTORY_TYPE_SHARED,
          __uuidof(IDWriteFactory),
          reinterpret_cast<IUnknown**>(factory.GetAddressOf()))) ||
      !factory) {
    return win32_deterministic_font_fallback();
  }

  ComPtr<IDWriteFontCollection> collection;
  if (FAILED(factory->GetSystemFontCollection(&collection)) || !collection) {
    return win32_deterministic_font_fallback();
  }

  std::vector<FontFaceDescriptor> records;
  const UINT32 family_count = collection->GetFontFamilyCount();
  records.reserve(family_count);

  for (UINT32 family_index = 0; family_index < family_count; ++family_index) {
    ComPtr<IDWriteFontFamily> family;
    if (FAILED(collection->GetFontFamily(family_index, &family)) || !family) {
      continue;
    }

    ComPtr<IDWriteLocalizedStrings> family_names;
    if (FAILED(family->GetFamilyNames(&family_names)) || !family_names) {
      continue;
    }

    const std::string family_name =
        preferred_localized_string(family_names.Get());
    if (family_name.empty()) {
      continue;
    }

    ComPtr<IDWriteFont> representative_font;
    (void)family->GetFirstMatchingFont(
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        &representative_font);

    records.push_back(FontFaceDescriptor{
        .font = FontDescriptor{.family = family_name},
        .postscript_name =
            directwrite_postscript_name(representative_font.Get(), family_name),
        .source = FontSource::platform,
        .path = std::string("directwrite://") + family_name,
    });
  }

  if (records.empty()) {
    return win32_deterministic_font_fallback();
  }

  return make_platform_font_discovery_result(
      PlatformFontDiscoveryBackend::direct_write,
      PlatformFontDiscoveryStatus::native_available,
      std::move(records));
}

} // namespace cgpui

#include "win32_system_appearance_internal.hpp"

#include <dwmapi.h>

#include <array>
#include <cwchar>
#include <string_view>

namespace cgpui {
namespace {

// Windows 10 20H1+ attribute; builds 1809-1909 used the undocumented 19.
constexpr DWORD immersive_dark_mode_attribute = 20;
constexpr DWORD legacy_immersive_dark_mode_attribute = 19;
constexpr std::wstring_view window_class_name = L"CGPUIWindow";

std::optional<DWORD> read_personalize_dword(const wchar_t* name) {
  DWORD value = 0;
  DWORD size = sizeof(value);
  if (RegGetValueW(
          HKEY_CURRENT_USER, win32_personalize_key, name, RRF_RT_REG_DWORD,
          nullptr, &value, &size) != ERROR_SUCCESS) {
    return std::nullopt;
  }
  return value;
}

float channel(DWORD argb, int shift) {
  return static_cast<float>((argb >> shift) & 0xFFU) / 255.0F;
}

BOOL CALLBACK apply_to_window(HWND hwnd, LPARAM lparam) {
  std::array<wchar_t, 64> name{};
  const int length =
      GetClassNameW(hwnd, name.data(), static_cast<int>(name.size()));
  if (length > 0 &&
      std::wstring_view(name.data(), static_cast<std::size_t>(length)) ==
          window_class_name) {
    (void)win32_apply_immersive_dark_mode(hwnd, lparam != 0);
  }
  return TRUE;
}

} // namespace

SystemAppearance win32_read_system_appearance() {
  SystemAppearance appearance{.supported = true, .backend = "win32"};
  // Missing values mean the Windows defaults: light apps, transparency on.
  const auto light = read_personalize_dword(L"AppsUseLightTheme");
  appearance.mode = light.value_or(1U) == 0U ? SystemAppearanceMode::dark
                                             : SystemAppearanceMode::light;
  appearance.transparency_effects_enabled =
      read_personalize_dword(L"EnableTransparency").value_or(1U) != 0U;

  DWORD colorization = 0;
  BOOL opaque_blend = FALSE;
  if (SUCCEEDED(DwmGetColorizationColor(&colorization, &opaque_blend))) {
    appearance.accent_color_available = true;
    appearance.accent_color = Color{
        channel(colorization, 16), channel(colorization, 8),
        channel(colorization, 0), 1.0F};
  }

  BOOL animation = TRUE;
  if (SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &animation, 0)) {
    appearance.client_area_animation_enabled = animation != FALSE;
  }
  HIGHCONTRASTW high_contrast{.cbSize = sizeof(HIGHCONTRASTW)};
  if (SystemParametersInfoW(
          SPI_GETHIGHCONTRAST, sizeof(high_contrast), &high_contrast, 0)) {
    appearance.high_contrast = (high_contrast.dwFlags & HCF_HIGHCONTRASTON) != 0;
  }
  return appearance;
}

std::optional<SystemAppearanceChangeSource> win32_appearance_change_source(
    UINT message,
    WPARAM wparam,
    LPARAM lparam) {
  if (message == WM_DWMCOLORIZATIONCOLORCHANGED) {
    return SystemAppearanceChangeSource::colorization;
  }
  if (message != WM_SETTINGCHANGE) return std::nullopt;
  if (wparam == SPI_SETCLIENTAREAANIMATION) {
    return SystemAppearanceChangeSource::client_area_animation;
  }
  if (wparam == SPI_SETHIGHCONTRAST) {
    return SystemAppearanceChangeSource::high_contrast;
  }
  const auto* area = reinterpret_cast<const wchar_t*>(lparam);
  if (area != nullptr && std::wcscmp(area, L"ImmersiveColorSet") == 0) {
    return SystemAppearanceChangeSource::color_set;
  }
  return std::nullopt;
}

bool win32_apply_immersive_dark_mode(HWND hwnd, bool dark) {
  if (hwnd == nullptr) return false;
  const BOOL value = dark ? TRUE : FALSE;
  if (SUCCEEDED(DwmSetWindowAttribute(
          hwnd, immersive_dark_mode_attribute, &value, sizeof(value)))) {
    return true;
  }
  return SUCCEEDED(DwmSetWindowAttribute(
      hwnd, legacy_immersive_dark_mode_attribute, &value, sizeof(value)));
}

void win32_apply_title_bar_appearance_to_thread_windows(bool dark) {
  EnumThreadWindows(GetCurrentThreadId(), apply_to_window, dark ? 1 : 0);
}

} // namespace cgpui

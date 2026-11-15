#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>

#include <cmath>
#include <iostream>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace {

constexpr const wchar_t* personalize =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";

int fail(int code, const char* message) {
  std::cerr << "win32 system appearance: " << message << '\n';
  return code;
}

DWORD personalize_dword(const wchar_t* name, DWORD fallback) {
  DWORD value = 0;
  DWORD size = sizeof(value);
  return RegGetValueW(HKEY_CURRENT_USER, personalize, name, RRF_RT_REG_DWORD,
                      nullptr, &value, &size) == ERROR_SUCCESS
      ? value
      : fallback;
}

bool close_to(float lhs, float rhs) { return std::fabs(lhs - rhs) < 0.002F; }

BOOL CALLBACK find_monitor(HWND hwnd, LPARAM lparam) {
  wchar_t name[64]{};
  GetClassNameW(hwnd, name, 64);
  if (std::wstring_view(name) == L"CGPUIAppearanceMonitor") {
    *reinterpret_cast<HWND*>(lparam) = hwnd;
    return FALSE;
  }
  return TRUE;
}

int check_direct_reads(const cgpui::SystemAppearance& appearance) {
  if (!appearance.supported || appearance.backend != "win32") return 10;
  const bool dark = personalize_dword(L"AppsUseLightTheme", 1U) == 0U;
  if ((appearance.mode == cgpui::SystemAppearanceMode::dark) != dark) return 11;
  if (appearance.transparency_effects_enabled !=
      (personalize_dword(L"EnableTransparency", 1U) != 0U)) return 12;
  DWORD color = 0;
  BOOL opaque = FALSE;
  if (SUCCEEDED(DwmGetColorizationColor(&color, &opaque))) {
    if (!appearance.accent_color_available ||
        !close_to(appearance.accent_color.r, ((color >> 16) & 0xFFU) / 255.0F) ||
        !close_to(appearance.accent_color.g, ((color >> 8) & 0xFFU) / 255.0F) ||
        !close_to(appearance.accent_color.b, (color & 0xFFU) / 255.0F) ||
        appearance.accent_color.a != 1.0F) return 13;
  }
  BOOL animation = TRUE;
  SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &animation, 0);
  if (appearance.client_area_animation_enabled != (animation != FALSE)) return 14;
  HIGHCONTRASTW contrast{.cbSize = sizeof(HIGHCONTRASTW)};
  SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0);
  if (appearance.high_contrast != ((contrast.dwFlags & HCF_HIGHCONTRASTON) != 0))
    return 15;
  return 0;
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) return fail(1, "create_platform_application failed");
  const cgpui::SystemAppearance appearance = (*app)->system_appearance();
  if (const int code = check_direct_reads(appearance); code != 0) {
    return fail(code, "query disagrees with direct registry/DWM/SPI reads");
  }

  std::vector<cgpui::SystemAppearanceChange> changes;
  (*app)->set_system_appearance_callback(
      [&](const cgpui::SystemAppearanceChange& change) {
        changes.push_back(change);
      });
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Appearance Test",
                              .size = cgpui::Size{320.0F, 240.0F}},
      [](const cgpui::PlatformEvent&) {});
  if (!window) return fail(2, "create_window failed");
  auto* hwnd = static_cast<HWND>(
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface()).hwnd);

  BOOL dark_title = FALSE;
  if (SUCCEEDED(DwmGetWindowAttribute(hwnd, 20, &dark_title, sizeof(dark_title))) &&
      (dark_title != FALSE) !=
          (appearance.mode == cgpui::SystemAppearanceMode::dark)) {
    return fail(3, "title bar dark mode does not match the system mode");
  }

  HWND monitor = nullptr;
  EnumThreadWindows(GetCurrentThreadId(), find_monitor,
                    reinterpret_cast<LPARAM>(&monitor));
  if (monitor == nullptr || IsWindowVisible(monitor) ||
      GetParent(monitor) != nullptr) {
    return fail(4, "hidden top-level appearance monitor window missing");
  }

  SendMessageW(monitor, WM_SETTINGCHANGE, 0,
               reinterpret_cast<LPARAM>(L"Environment"));
  if (!changes.empty()) return fail(5, "unrelated WM_SETTINGCHANGE notified");

  SendMessageW(monitor, WM_SETTINGCHANGE, 0,
               reinterpret_cast<LPARAM>(L"ImmersiveColorSet"));
  SendMessageW(monitor, WM_SETTINGCHANGE, SPI_SETCLIENTAREAANIMATION, 0);
  SendMessageW(monitor, WM_SETTINGCHANGE, SPI_SETHIGHCONTRAST, 0);
  SendMessageW(monitor, WM_DWMCOLORIZATIONCOLORCHANGED, 0, 0);
  using Source = cgpui::SystemAppearanceChangeSource;
  const Source expected[]{Source::color_set, Source::client_area_animation,
                          Source::high_contrast, Source::colorization};
  if (changes.size() != 4) return fail(6, "expected four notifications");
  cgpui::SystemAppearance previous = appearance;
  for (std::size_t index = 0; index < 4; ++index) {
    if (changes[index].source != expected[index] ||
        check_direct_reads(changes[index].appearance) != 0) {
      return fail(7, "notification carried a wrong source or snapshot");
    }
    // Synthetic messages do not change settings, so `changed` stays false
    // unless the user really toggled a setting while the test ran.
    if (changes[index].changed && changes[index].appearance == previous) {
      return fail(8, "changed flag set for an identical snapshot");
    }
    previous = changes[index].appearance;
  }

  (*app)->set_system_appearance_callback({});
  SendMessageW(monitor, WM_SETTINGCHANGE, 0,
               reinterpret_cast<LPARAM>(L"ImmersiveColorSet"));
  if (changes.size() != 4) return fail(9, "cleared callback still notified");
  return 0;
}

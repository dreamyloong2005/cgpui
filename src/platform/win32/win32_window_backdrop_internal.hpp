#pragma once

#include "cgpui/platform/platform_appearance.hpp"
#include "cgpui/platform/platform_window_backdrop.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>

namespace cgpui {

// DWMWA_SYSTEMBACKDROP_TYPE and its DWM_SYSTEMBACKDROP_TYPE values, available
// from Windows 11 build 22621.
inline constexpr DWORD win32_system_backdrop_attribute = 38;
inline constexpr DWORD win32_backdrop_type_none = 1;
inline constexpr DWORD win32_backdrop_type_main_window = 2;
inline constexpr DWORD win32_backdrop_type_transient_window = 3;
inline constexpr DWORD win32_backdrop_type_tabbed_window = 4;
inline constexpr std::uint32_t win32_system_backdrop_min_build = 22621;

struct Win32BackdropInputs {
  WindowBackdrop requested = WindowBackdrop::none;
  bool transparent_surface = false;
  std::uint32_t os_build = 0;
  bool layered = false;
  bool transparency_effects_enabled = true;
  bool client_title_bar = false;
};

struct Win32BackdropPlan {
  PlatformWindowBackdropState state;
  bool touch_dwm = false;
  DWORD backdrop_type = win32_backdrop_type_none;
  bool extend_frame = false;
};

// Pure decision: which DWM backdrop type to set and what to report.
[[nodiscard]] Win32BackdropPlan win32_plan_window_backdrop(
    const Win32BackdropInputs& inputs);

// Real OS build from RtlGetVersion (GetVersionEx is manifest-dependent).
[[nodiscard]] std::uint32_t win32_os_build_number();

// Window property holding the window's Win32WindowBackdrop.
inline constexpr const wchar_t* win32_window_backdrop_property = L"CGPUIWindowBackdrop";

// Per-window backdrop owner. It remembers the last request so a system
// appearance change can re-evaluate it, and publishes itself as a window
// property so the appearance monitor can find it by HWND.
class Win32WindowBackdrop {
 public:
  Win32WindowBackdrop() = default;
  ~Win32WindowBackdrop();
  Win32WindowBackdrop(const Win32WindowBackdrop&) = delete;
  Win32WindowBackdrop& operator=(const Win32WindowBackdrop&) = delete;

  PlatformWindowBackdropState apply(
      HWND hwnd,
      WindowBackdrop backdrop,
      bool transparent_surface,
      bool layered,
      bool client_title_bar);
  PlatformWindowBackdropState reevaluate(const SystemAppearance& appearance);
  void detach();
  [[nodiscard]] const PlatformWindowBackdropState& state() const;
  [[nodiscard]] bool frame_extended() const { return frame_extended_; }

 private:
  PlatformWindowBackdropState apply_plan(const Win32BackdropPlan& plan);

  HWND hwnd_ = nullptr;
  WindowBackdrop requested_ = WindowBackdrop::none;
  bool transparent_surface_ = false;
  bool layered_ = false;
  bool client_title_bar_ = false;
  bool frame_extended_ = false;
  PlatformWindowBackdropState state_{
      .backend = "win32", .reason = "backdrop not applied yet"};
};

// Re-evaluates every CGPUI window backdrop on this thread against
// `appearance`. The appearance monitor calls it for each change message.
void win32_reevaluate_thread_window_backdrops(
    const SystemAppearance& appearance);

} // namespace cgpui

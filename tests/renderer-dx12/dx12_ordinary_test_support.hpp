#pragma once

#include "dx12_popup_test_support.hpp"
#include "cgpui/platform/platform.hpp"

namespace cgpui_test {

inline HWND ordinary_hwnd(const cgpui::PlatformWindow& window) {
  const auto surface = window.native_surface();
  const auto* native = std::get_if<cgpui::Win32SurfaceHandle>(&surface);
  return native ? static_cast<HWND>(native->hwnd) : nullptr;
}

// A separate, process-owned green HWND is the desktop-alpha oracle. The
// tested ordinary window itself is always created through the public API.
class OrdinaryDesktop {
 public:
  OrdinaryDesktop() {
    if (!desktop_.ready() || !dx12_popup_desktop_ready() ||
        !SystemParametersInfoW(SPI_GETWORKAREA, 0, &work_, 0) ||
        work_.right - work_.left < 500 || work_.bottom - work_.top < 380) return;
    brush_ = CreateSolidBrush(RGB(0, 255, 0));
    WNDCLASSEXW type{.cbSize = sizeof(WNDCLASSEXW), .lpfnWndProc = DefWindowProcW,
        .hInstance = GetModuleHandleW(nullptr), .hbrBackground = brush_,
        .lpszClassName = L"CgpuiOrdinaryAlphaTestBackdrop"};
    registered_ = RegisterClassExW(&type) != 0;
    if (!registered_) return;
    hwnd_ = CreateWindowExW(WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
        type.lpszClassName, L"", WS_POPUP | WS_VISIBLE, work_.left + 16, work_.top + 16,
        464, 344, nullptr, nullptr, type.hInstance, nullptr);
    pump_messages_for(80);
  }
  ~OrdinaryDesktop() {
    if (hwnd_) DestroyWindow(hwnd_);
    if (registered_) UnregisterClassW(L"CgpuiOrdinaryAlphaTestBackdrop", GetModuleHandleW(nullptr));
    if (brush_) DeleteObject(brush_);
  }
  OrdinaryDesktop(const OrdinaryDesktop&) = delete;
  OrdinaryDesktop& operator=(const OrdinaryDesktop&) = delete;

  [[nodiscard]] bool valid() const { return hwnd_ != nullptr; }
  [[nodiscard]] cgpui::WindowDescriptor descriptor(bool transparent) const {
    return {.title = "DX12 ordinary native-menu acceptance", .size = {240, 160},
        .position = cgpui::Point{float(work_.left + 48), float(work_.top + 48)},
        .chrome = {.transparent_background = transparent}};
  }
  bool place(cgpui::PlatformWindow& window) const {
    const HWND hwnd = ordinary_hwnd(window);
    if (!hwnd || !SetWindowPos(hwnd, HWND_TOPMOST, work_.left + 48, work_.top + 48,
                              400, 280, SWP_SHOWWINDOW)) return false;
    SetActiveWindow(hwnd);
    SetFocus(hwnd);
    pump_messages_for(100);
    RECT target{}, backing{};
    const LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    const LONG_PTR extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    return GetWindowRect(hwnd, &target) && GetWindowRect(hwnd_, &backing) &&
        target.left > backing.left && target.top > backing.top &&
        target.right < backing.right && target.bottom < backing.bottom &&
        (style & WS_CAPTION) == WS_CAPTION && (style & WS_THICKFRAME) &&
        !(style & (WS_POPUP | WS_CHILD)) &&
        !(extended & (WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_NOACTIVATE)) &&
        GetActiveWindow() == hwnd && GetFocus() == hwnd &&
        expect_screen_color({0, 255, 0}, {backing.left + 8, backing.top + 8});
  }

 private:
  Win32DesktopTestLease desktop_;
  RECT work_{};
  HBRUSH brush_ = nullptr;
  HWND hwnd_ = nullptr;
  bool registered_ = false;
};

inline cgpui::NativeMenuModel ordinary_native_menu() {
  return {.items = {{.kind = cgpui::NativeMenuItemKind::submenu, .title = "&File",
      .children = {{.title = "&Open", .action_name = "file.open"}}},
      {.title = "Native text item", .action_name = "other"}}};
}

inline bool ordinary_install_menu(cgpui::PlatformApplication& app,
                                   cgpui::PlatformWindow& window) {
  const auto result = app.install_native_menu(ordinary_native_menu());
  pump_messages_for(80);
  HMENU menu = GetMenu(ordinary_hwnd(window));
  return result.supported && menu && GetMenuItemCount(menu) == 2 && GetSubMenu(menu, 0);
}

inline cgpui::Result<std::unique_ptr<cgpui::Renderer>> ordinary_renderer(
    const cgpui::PlatformWindow& window, bool transparent) {
  const auto state = window.state();
  return cgpui::create_dx12_renderer({.native_surface = window.native_surface(),
      .framebuffer_size = state.framebuffer_size, .scale = state.scale,
      .transparent_background = transparent});
}

// Read the actual OS version, not a manifest-dependent compatibility version.
// On supported builds every positive oracle remains mandatory: a factory
// failure must never be reclassified as a successful skip.
inline std::optional<int> ordinary_alpha_os_gate(const cgpui::PlatformWindow& window) {
  using VersionFunction = LONG(WINAPI*)(OSVERSIONINFOW*);
  const auto query = reinterpret_cast<VersionFunction>(
      GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"));
  OSVERSIONINFOW version{sizeof(version)};
  if (!query || query(&version) < 0)
    return dx12_fail(80, "cannot establish OS build; refusing to skip ordinary alpha acceptance");
  std::cout << "ORDINARY_ALPHA_OS build=" << version.dwBuildNumber << std::endl;
  if (version.dwBuildNumber >= 26100) return std::nullopt;
  const HWND hwnd = ordinary_hwnd(window);
  const HMENU menu = GetMenu(hwnd);
  const auto style = GetWindowLongPtrW(hwnd, GWL_STYLE);
  const auto extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  const auto rejected = ordinary_renderer(window, true);
  if (rejected || rejected.error().code != cgpui::ErrorCode::unsupported_platform)
    return dx12_fail(81, "pre-26100 ordinary alpha creation did not reject as unsupported_platform");
  if (ordinary_hwnd(window) != hwnd || GetMenu(hwnd) != menu ||
      GetWindowLongPtrW(hwnd, GWL_STYLE) != style || GetWindowLongPtrW(hwnd, GWL_EXSTYLE) != extended)
    return dx12_fail(82, "pre-26100 ordinary alpha refusal changed the native window/menu");
  std::cout << "SKIP ordinary alpha positive cases: Windows build " << version.dwBuildNumber
            << " < 26100; unsupported_platform verified" << std::endl;
  return 0;
}

inline bool ordinary_composition(const cgpui::RendererSurfaceComposition& report, bool alpha) {
  return report.supported && report.transparent_requested == alpha &&
      report.composite_alpha == (alpha ? cgpui::RendererCompositeAlpha::pre_multiplied
                                      : cgpui::RendererCompositeAlpha::opaque);
}

} // namespace cgpui_test

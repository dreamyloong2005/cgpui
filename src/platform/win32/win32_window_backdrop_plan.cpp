#include "win32_window_backdrop_internal.hpp"

#include <string>

namespace cgpui {
namespace {

WindowBackdrop windows_material(WindowBackdrop requested) {
  // DWM has no generic blur attribute; acrylic is its blur-behind material.
  return requested == WindowBackdrop::blur ? WindowBackdrop::acrylic
                                           : requested;
}

DWORD backdrop_type(WindowBackdrop material) {
  switch (material) {
    case WindowBackdrop::mica:
      return win32_backdrop_type_main_window;
    case WindowBackdrop::mica_alt:
      return win32_backdrop_type_tabbed_window;
    case WindowBackdrop::acrylic:
    case WindowBackdrop::blur:
      return win32_backdrop_type_transient_window;
    case WindowBackdrop::none:
      break;
  }
  return win32_backdrop_type_none;
}

std::string type_mechanism(DWORD type) {
  return "DWMWA_SYSTEMBACKDROP_TYPE=" + std::to_string(type);
}

} // namespace

Win32BackdropPlan win32_plan_window_backdrop(const Win32BackdropInputs& inputs) {
  Win32BackdropPlan plan{.state = PlatformWindowBackdropState{
                             .transparent_surface = inputs.transparent_surface,
                             .backend = "win32",
                             .requested = inputs.requested}};
  PlatformWindowBackdropState& state = plan.state;
  if (inputs.os_build < win32_system_backdrop_min_build) {
    state.reason = "DWMWA_SYSTEMBACKDROP_TYPE requires Windows build 22621; "
                   "this is build " + std::to_string(inputs.os_build);
    return plan;
  }
  state.supported = true;
  plan.touch_dwm = true;
  state.mechanism = type_mechanism(win32_backdrop_type_none);
  if (inputs.requested == WindowBackdrop::none) return plan;
  if (!inputs.transparency_effects_enabled) {
    state.reason = "transparency effects are turned off in Windows settings";
    return plan;
  }
  const WindowBackdrop material = windows_material(inputs.requested);
  plan.backdrop_type = backdrop_type(material);
  state.title_bar = material;
  state.mechanism = type_mechanism(plan.backdrop_type);
  if (!inputs.transparent_surface && inputs.client_title_bar) {
    state.title_bar = WindowBackdrop::none;
    state.reason = "renderer surface does not composite with alpha and the "
                   "system title bar is replaced by a client title bar; the "
                   "material is not visible";
    return plan;
  }
  if (!inputs.transparent_surface) {
    state.reason = "renderer surface does not composite with alpha; the "
                   "material is drawn on the title bar only";
    return plan;
  }
  plan.extend_frame = true;
  state.applied = material;
  state.mechanism += " + DwmExtendFrameIntoClientArea(-1)";
  return plan;
}

std::uint32_t win32_os_build_number() {
  using RtlGetVersionFunction = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
  static const std::uint32_t build = [] {
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    const auto rtl_get_version = ntdll == nullptr
        ? nullptr
        : reinterpret_cast<RtlGetVersionFunction>(
              reinterpret_cast<void*>(GetProcAddress(ntdll, "RtlGetVersion")));
    RTL_OSVERSIONINFOW info{};
    info.dwOSVersionInfoSize = sizeof(info);
    if (rtl_get_version == nullptr || rtl_get_version(&info) != 0) return 0U;
    return static_cast<std::uint32_t>(info.dwBuildNumber);
  }();
  return build;
}

} // namespace cgpui

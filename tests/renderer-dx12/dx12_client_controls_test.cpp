#include "dx12_client_controls_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return dx12_fail(1, "client controls fixture unavailable");
  unsigned closes = 0, resizes = 0;
  WindowCloseRequested close_event;
  const auto descriptor = client_alpha_descriptor(desktop);
  auto window = (*app)->create_window(descriptor, [&](const PlatformEvent& event) {
    if (const auto* close = std::get_if<WindowCloseRequested>(&event)) { ++closes; close_event = *close; }
    if (std::holds_alternative<WindowResized>(event)) ++resizes;
  });
  if (!window || !desktop.place(**window)) return dx12_fail(2, "client controls window unavailable");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const auto style = GetWindowLongPtrW(hwnd, GWL_STYLE);
  const auto extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  RECT restored_bounds{};
  if (!GetWindowRect(hwnd, &restored_bounds)) return dx12_fail(3, "client controls rectangle unavailable");
  auto renderer = ordinary_renderer(**window, true);
  if (!renderer || !controls_paint(**renderer, **window, false))
    return dx12_fail(4, "initial client controls alpha failed");
  const auto initial_resizes = resizes;
  if (!control_button(**window, HTMAXBUTTON) || !controls_wait([&] { return IsZoomed(hwnd) != FALSE; }) ||
      resizes <= initial_resizes || (*window)->frame_metrics().client_resize_border.top != 0 ||
      !controls_paint(**renderer, **window, true, false))
    return dx12_fail(5, "client HTMAXBUTTON did not maximize or update alpha GPU geometry");
  // A maximized window no longer fits the green desktop fixture; only GPU alpha
  // is asserted there. Restore re-establishes the physical desktop oracle.
  if (!control_button(**window, HTMAXBUTTON) || !controls_wait([&] { return !IsZoomed(hwnd); }) ||
      !desktop.place(**window) || !controls_paint(**renderer, **window, false) ||
      GetWindowLongPtrW(hwnd, GWL_STYLE) != style || GetWindowLongPtrW(hwnd, GWL_EXSTYLE) != extended)
    return dx12_fail(6, "client maximize-button restore lost desktop alpha or native topology");

  const auto minimize = (*window)->perform_window_action({.action = PlatformWindowAction::minimize});
  const bool minimized = controls_wait([&] { return IsIconic(hwnd) != FALSE; });
  const auto display = (*window)->lifecycle_state().display_state;
  std::cout << "CLIENT_MINIMIZE supported=" << minimize.supported << " performed=" << minimize.performed
            << " iconic=" << IsIconic(hwnd) << " display=" << static_cast<int>(display) << '\n';
  if (!minimize.supported || !minimize.performed || !minimized || display != PlatformWindowDisplayState::minimized)
    return dx12_fail(7, "client minimize action did not change the real native state");
  if (!(*window)->request_display_state(PlatformWindowDisplayState::normal) ||
      !controls_wait([&] { return !IsIconic(hwnd) && !IsZoomed(hwnd); }) ||
      !desktop.place(**window) || !controls_paint(**renderer, **window, true))
    return dx12_fail(8, "restoring minimized client window lost desktop alpha");
  if (!control_button(**window, HTCLOSE) || !controls_wait([&] { return closes != 0; }) ||
      closes != 1 || close_event.source != WindowCloseRequestSource::window_manager ||
      !close_event.sequence || !IsWindow(hwnd) || !(*window)->state().close_requested)
    return dx12_fail(9, "client close button bypassed close request policy or destroyed the HWND");
  if (!(*window)->resolve_close_request(PlatformWindowCloseResolution::cancel) ||
      (*window)->state().close_requested || !controls_paint(**renderer, **window, false))
    return dx12_fail(10, "cancelled client close request broke alpha presentation");
  renderer->reset(); window->reset();

  // Nonresizable is an initial descriptor choice, never a live style workaround.
  auto fixed_descriptor = descriptor;
  fixed_descriptor.chrome.resizable = false;
  auto fixed = (*app)->create_window(fixed_descriptor, [](const PlatformEvent&) {});
  if (!fixed) return dx12_fail(11, "initial fixed-size client unavailable");
  const HWND fixed_hwnd = ordinary_hwnd(**fixed);
  if (!SetWindowPos(fixed_hwnd, HWND_TOPMOST, restored_bounds.left, restored_bounds.top,
      restored_bounds.right - restored_bounds.left, restored_bounds.bottom - restored_bounds.top, SWP_SHOWWINDOW))
    return dx12_fail(12, "fixed-size client placement failed");
  pump_messages_for(80);
  if (GetMenu(fixed_hwnd) || (GetWindowLongPtrW(fixed_hwnd, GWL_STYLE) & WS_CAPTION) != WS_CAPTION ||
      (GetWindowLongPtrW(fixed_hwnd, GWL_STYLE) & (WS_POPUP | WS_CHILD)) ||
      (GetWindowLongPtrW(fixed_hwnd, GWL_EXSTYLE) & (WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP)))
    return dx12_fail(13, "fixed-size client silently changed native topology");
  auto fixed_renderer = ordinary_renderer(**fixed, true);
  if (!fixed_renderer || !controls_paint(**fixed_renderer, **fixed, true) || !fixed_top_hits(**fixed))
    return dx12_fail(14, "fixed-size client alpha or top caption/client hit ownership differs");
  std::cout << "CLIENT_CONTROLS max/restore/minimize/close-policy/fixed-size passed; no physical Snap claim\n";
  return 0;
}

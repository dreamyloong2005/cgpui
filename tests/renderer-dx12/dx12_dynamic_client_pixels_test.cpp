#include "dx12_dynamic_client_test_support.hpp"
#include "dx12_client_controls_support.hpp"

int main() {
  using namespace cgpui;
  using namespace cgpui_test;
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return dx12_fail(1, "dynamic client fixture unavailable");
  auto descriptor = client_alpha_descriptor(desktop);
  descriptor.chrome.transparent_background = false;
  auto window = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!window || !desktop.place(**window)) return dx12_fail(2, "fresh opaque client window unavailable");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  ClientAlphaTopology initial;
  client_alpha_regions(**window);
  if (!initial.capture(**window) || !client_alpha_hits(**window)) return dx12_fail(3, "client geometry/hits unavailable");
  auto renderer = ordinary_renderer(**window, false);
  if (!renderer) return dx12_fail(4, renderer.error().message.c_str());
  const auto identity = (*renderer)->backend_info();
  unsigned step = 0;
  for (const bool alpha : {false, true, false, true}) {
    const bool smaller = (++step % 2) == 0;
    if (!(*renderer)->set_transparent_surface(alpha) ||
        !SetWindowPos(initial.hwnd, nullptr, 0, 0, smaller ? 376 : 400, smaller ? 248 : 280,
                      SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE)) return dx12_fail(5, "dynamic client toggle/resize failed");
    pump_messages_for(60);
    if (!dynamic_client_resize(**renderer, **window) || !dynamic_client_paint(**renderer, **window, smaller, alpha))
      return dx12_fail(6, "client title-row/body GPU/desktop opacity or hit regions differ");
    if (ordinary_hwnd(**window) != initial.hwnd || GetMenu(initial.hwnd) ||
        GetWindowLongPtrW(initial.hwnd, GWL_STYLE) != initial.style ||
        GetWindowLongPtrW(initial.hwnd, GWL_EXSTYLE) != initial.extended ||
        (*renderer)->backend_info().device_generation != identity.device_generation ||
        (*renderer)->backend_info().adapter_luid != identity.adapter_luid)
      return dx12_fail(7, "dynamic client presentation changed HWND/style/adapter/epoch");
  }
  // Native controls remain Windows actions, not a simulated renderer fallback.
  if (!control_button(**window, HTMAXBUTTON) || !controls_wait([&] { return IsZoomed(initial.hwnd) != FALSE; }) ||
      !controls_paint(**renderer, **window, false, false)) return dx12_fail(8, "dynamic client maximize/GPU paint failed");
  if (!control_button(**window, HTMAXBUTTON) || !controls_wait([&] { return !IsZoomed(initial.hwnd); }) ||
      !desktop.place(**window) || !dynamic_client_resize(**renderer, **window) ||
      !dynamic_client_paint(**renderer, **window, true, true)) return dx12_fail(9, "dynamic client restore lost desktop alpha");
  return 0;
}

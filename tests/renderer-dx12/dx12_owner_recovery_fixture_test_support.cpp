#include "dx12_owner_recovery_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"

namespace cgpui_test {
ManagedRecoveryWindow::ManagedRecoveryWindow() : application(cgpui::create_platform_application()) {
  if (!application) return;
  auto made = (*application)->create_window({.title = "DX12 owner recovery", .size = {96, 96},
      .chrome = {.titlebar_visible = false, .decorations = false, .resizable = false}},
      [](const cgpui::PlatformEvent&) {});
  if (!made) return;
  window = std::move(*made);
  const auto state = window->state();
  auto native = cgpui::create_dx12_renderer({.native_surface = window->native_surface(),
      .framebuffer_size = state.framebuffer_size, .scale = state.scale});
  if (native) renderer = std::move(*native);
}
bool owner_recovery_pump() {
  MSG message{};
  bool recovery_dispatched = false;
  for (unsigned count = 0; count != 32 && PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE); ++count) {
    if (message.message == WM_QUIT) { PostQuitMessage(static_cast<int>(message.wParam)); break; }
    TranslateMessage(&message);
    DispatchMessageW(&message);
    recovery_dispatched = recovery_dispatched || message.message == WM_APP + 9;
  }
  return recovery_dispatched;
}
bool owner_recovery_paint(cgpui::Renderer& renderer, bool upload) {
  using namespace cgpui;
  const ImageAsset image{.id = {8670}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {0, 255, 0, 255}}};
  auto frame = wait_for_test_frame(renderer, {.message_owner = std::this_thread::get_id()});
  if (!frame) return false;
  (*frame)->clear({0, 0, 1, 1});
  if (upload) (*frame)->upload_image(image);
  (*frame)->draw_image({.bounds = {{0, 0}, {96, 96}}, .asset = describe_image_asset(image)});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, {0, 255, 0, 255});
}
} // namespace cgpui_test

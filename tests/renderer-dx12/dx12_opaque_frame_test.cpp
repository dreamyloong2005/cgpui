#include "../support/frame_wait_test_support.hpp"
#include "cgpui/renderer/renderer_backend.hpp"
#include "cgpui/renderer/renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <array>
#include <cstdlib>
#include <iostream>

namespace {
struct Window {
  HWND hwnd = CreateWindowExW(0, L"STATIC", L"DX12 opaque regression",
      WS_POPUP, 64, 64, 64, 64, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  ~Window() { if (hwnd) DestroyWindow(hwnd); }
};
bool pixel(const cgpui::RendererFramePixels* capture, unsigned x, unsigned y,
           std::array<unsigned char, 4> expected) {
  if (!capture || !capture->valid()) return false;
  auto actual = capture->pixel_rgba8(x, y);
  if (!actual) return false;
  for (unsigned i = 0; i != 4; ++i) {
    if (std::abs(int((*actual)[i]) - int(expected[i])) > 2) return false;
  }
  return true;
}
int fail(int code, const char* message) {
  std::cerr << "DX12 opaque regression: " << message << '\n';
  return code;
}
}

int main() {
  Window window;
  if (!window.hwnd) return fail(1, "CreateWindowExW failed");
  cgpui::RenderSurfaceDescriptor descriptor{
      .native_surface = cgpui::Win32SurfaceHandle{
          .hinstance = GetModuleHandleW(nullptr), .hwnd = window.hwnd},
      .framebuffer_size = {64, 64}, .scale = {1}};
  auto renderer = cgpui::create_dx12_renderer(descriptor);
  if (!renderer) return fail(2, renderer.error().message.c_str());
  const auto info = (*renderer)->backend_info();
  std::cout << "DX12 adapter: " << info.adapter_name << ", vendor=" << info.vendor_id
            << ", software=" << info.software_adapter << '\n';
  if (!(*renderer)->popup_windows_supported()) return fail(3, "parent renderer must allow native popup attempts");
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return fail(4, frame.error().message.c_str());
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_rect({.rect = {{8, 8}, {16, 16}}, .color = {1, 0, 0, 1}});
  if (!(*frame)->request_pixel_capture()) return fail(5, "capture unsupported");
  auto presented = (*frame)->present();
  if (!presented) return fail(6, presented.error().message.c_str());
  if (!pixel((*renderer)->last_frame_pixels(), 12, 12, {255, 0, 0, 255}) ||
      !pixel((*renderer)->last_frame_pixels(), 40, 40, {0, 0, 255, 255})) {
    return fail(7, "captured clear/rectangle pixels differ");
  }
  frame->reset();
  // Exercise hidden and visible HWNDs; readback is not a desktop screenshot.
  ShowWindow(window.hwnd, SW_SHOWNOACTIVATE);
  UpdateWindow(window.hwnd);
  MSG message{};
  while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
  if (!IsWindowVisible(window.hwnd) || IsIconic(window.hwnd)) {
    return fail(16, "visible HWND setup failed");
  }
  auto resized = (*renderer)->resize({80, 48}, {1});
  if (!resized) return fail(8, resized.error().message.c_str());
  frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return fail(9, frame.error().message.c_str());
  (*frame)->clear({0, 1, 0, 1});
  auto capture_requested = (*frame)->request_pixel_capture();
  if (!capture_requested) return fail(10, capture_requested.error().message.c_str());
  auto resized_present = (*frame)->present();
  if (!resized_present) return fail(10, resized_present.error().message.c_str());
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || pixels->width != 80 || pixels->height != 48 ||
      !pixel(pixels, 40, 20, {0, 255, 0, 255})) return fail(11, "resize pixels differ");
  frame->reset();
  ShowWindow(window.hwnd, SW_HIDE);
  frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return fail(12, frame.error().message.c_str());
  (*frame)->draw_backdrop_blur({});
  const auto empty_backdrop = (*frame)->present();
  if (!empty_backdrop) {
    return fail(13, "empty backdrop must be a valid no-op");
  }
  renderer->reset();
  descriptor.transparent_background = true;
  if (cgpui::create_dx12_renderer(descriptor)) return fail(14, "transparent surface accepted");
  descriptor.transparent_background = false;
  std::get<cgpui::Win32SurfaceHandle>(descriptor.native_surface).composition = true;
  if (cgpui::create_dx12_renderer(descriptor)) return fail(15, "mismatched HWND composition style accepted");
  return 0;
}

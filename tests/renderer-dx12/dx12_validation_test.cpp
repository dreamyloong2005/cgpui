#include "dx12_test_support.hpp"

#include <limits>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "native window failed");
  const auto good = window.descriptor();
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float inf = std::numeric_limits<float>::infinity();
  for (const Size size : {Size{0, 64}, {-1, 64}, {nan, 64}, {64, inf}, {1e20F, 64}}) {
    auto invalid = good;
    invalid.framebuffer_size = size;
    if (create_dx12_renderer(invalid)) return dx12_fail(2, "invalid framebuffer accepted");
  }
  for (const float scale : {0.0F, -1.0F, nan, inf}) {
    auto invalid = good;
    invalid.scale = {scale};
    if (create_dx12_renderer(invalid)) return dx12_fail(3, "invalid scale accepted");
  }
  auto invalid = good;
  invalid.native_surface = Win32SurfaceHandle{};
  if (create_dx12_renderer(invalid)) return dx12_fail(4, "null HWND accepted");
  invalid.native_surface = MetalSurfaceHandle{};
  if (create_dx12_renderer(invalid)) return dx12_fail(5, "non-Windows surface accepted");

  auto renderer = create_dx12_renderer(good);
  if (!renderer) return dx12_fail(6, renderer.error().message.c_str());
  if ((*renderer)->resize({nan, 32}, {1}) || (*renderer)->resize({64, 64}, {inf})) {
    return dx12_fail(7, "invalid resize accepted");
  }
  if ((*renderer)->set_transparent_surface(true)) return dx12_fail(8, "unimplemented alpha accepted");
  const auto composition = (*renderer)->set_transparent_surface(false);
  if (!composition || composition->alpha_composited()) return dx12_fail(9, "opaque surface misreported");
  auto frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(10, "invalid request poisoned valid renderer");
  (*frame)->clear({0, 0, 1, 1});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 4, 4, {0, 0, 255, 255})) {
    return dx12_fail(11, "valid frame after rejected request failed");
  }
  return 0;
}

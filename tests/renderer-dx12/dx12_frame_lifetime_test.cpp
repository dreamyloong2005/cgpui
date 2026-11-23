#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "native window failed");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  auto first = (*renderer)->begin_frame();
  if (!first) return dx12_fail(3, first.error().message.c_str());
  if ((*renderer)->begin_frame()) return dx12_fail(4, "overlapping frame accepted");
  if ((*renderer)->resize({80, 40}, {1})) return dx12_fail(5, "resize during frame accepted");
  first->reset();
  if (!(*renderer)->resize({64, 64}, {1})) return dx12_fail(6, "abandoned frame retained ownership");

  first = (*renderer)->begin_frame();
  if (!first) return dx12_fail(7, "frame after abandonment failed");
  (*first)->clear({0, 0, 0, 1});
  if (!(*first)->present()) return dx12_fail(8, "initial present failed");
  if ((*first)->present()) return dx12_fail(9, "frame presented twice");
  auto second = (*renderer)->begin_frame();
  if (!second) return dx12_fail(10, "finished frame blocks next frame");
  first->reset();
  if ((*renderer)->begin_frame()) return dx12_fail(11, "old frame destructor released new frame");
  (*second)->clear({0, 1, 0, 1});
  if (!(*second)->request_pixel_capture() || !(*second)->present()) {
    return dx12_fail(12, "next frame failed after previous destruction");
  }
  if (!dx12_pixel((*renderer)->last_frame_pixels(), 1, 1, {0, 255, 0, 255})) {
    return dx12_fail(13, "frame sequencing corrupted pixels");
  }
  second->reset();

  auto orphan = (*renderer)->begin_frame();
  if (!orphan) return dx12_fail(14, "orphan frame creation failed");
  renderer->reset();
  (*orphan)->clear({1, 0, 0, 1});
  if ((*orphan)->present()) return dx12_fail(15, "frame accepted after renderer destruction");
  orphan->reset();
  return 0;
}

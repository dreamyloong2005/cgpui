#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window first_window;
  Dx12Window second_window;
  if (!first_window.hwnd || !second_window.hwnd) return dx12_fail(1, "native windows failed");
  auto first = create_dx12_renderer(first_window.descriptor());
  if (!first) return dx12_fail(2, first.error().message.c_str());
  auto frame = cgpui_test::wait_for_test_frame(**first);
  if (!frame || !(*frame)->present()) return dx12_fail(3, "initial frame failed");
  frame->reset();
  auto second = create_dx12_renderer(second_window.descriptor());
  if (!second) return dx12_fail(4, second.error().message.c_str());
  for (unsigned iteration = 0; iteration < 3; ++iteration) {
    for (Renderer* renderer : {first->get(), second->get()}) {
      frame = cgpui_test::wait_for_test_frame(*renderer);
      if (!frame) return dx12_fail(5, frame.error().message.c_str());
      (*frame)->clear({0, 1, 0, 1});
      if (!(*frame)->request_pixel_capture()) return dx12_fail(6, "capture request failed");
      auto result = (*frame)->present();
      if (!result) return dx12_fail(7, result.error().message.c_str());
      if (!dx12_pixel(renderer->last_frame_pixels(), 4, 4, {0, 255, 0, 255})) {
        return dx12_fail(8, "concurrent renderer corrupted pixels");
      }
      frame->reset();
    }
  }
  return 0;
}

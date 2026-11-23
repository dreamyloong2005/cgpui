#include "../support/frame_wait_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
using Microsoft::WRL::ComPtr;

namespace {
void paint_resources(RenderFrame& frame, const ImageAsset& image) {
  frame.clear({0, 0, 1, 1});
  frame.draw_image({.bounds = {{4, 4}, {32, 32}}, .asset = describe_image_asset(image)});
  frame.draw_box_shadow({.rect = {{40, 40}, {16, 16}},
      .shadow = {.color = {0, 0, 0, 1}, .offset = {-8, -8}, .blur_radius = 4}});
  frame.draw_backdrop_blur({.rect = {{8, 8}, {40, 40}}, .blur_radius = 4});
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done) return dx12_fail(1, "device-removal watchdog unavailable");
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(2, "device-removal window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(3, renderer.error().message.c_str());
  const auto info = (*renderer)->backend_info();
  if (!info.adapter_luid) return dx12_fail(4, "device-removal adapter LUID unavailable");
  auto native = dx12_acquire_native_device(info);
  if (!native)
    return dx12_fail(5, "same-adapter ID3D12Device5 unavailable; removal was not tested");
  ComPtr<ID3D12InfoQueue> messages;
  (void)native.As(&messages);
  if (dx12_recovery_debug_requested() && !messages)
    return dx12_fail(14, "requested device-removal debug validation has no InfoQueue");
  const ImageAsset image{.id = {8301}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  auto warmup = cgpui_test::wait_for_test_frame(**renderer);
  if (!warmup) return dx12_fail(6, "device-removal warmup begin failed");
  (*warmup)->upload_image(image);
  paint_resources(**warmup, image);
  if (!(*warmup)->request_pixel_capture() || !(*warmup)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 60, 60, {0, 0, 255, 255}))
    return dx12_fail(7, "device-removal resource warmup failed");
  warmup->reset();
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(8, "device-removal frame begin failed");
  paint_resources(**frame, image); // Reuses the previously realized image snapshot.
  if (!(*frame)->request_pixel_capture()) return dx12_fail(9, "removal capture request failed");
  if (messages) messages->ClearStoredMessages();
  // Unlike recovery tests, keep Device5 AND InfoQueue alive through failed
  // operations and renderer destruction: replacement must remain blocked.
  native->RemoveDevice();
  if (SUCCEEDED(native->GetDeviceRemovedReason()))
    return dx12_fail(10, "native device was not actually removed");
  const auto presented = (*frame)->present();
  if (presented || presented.error().code != ErrorCode::renderer_initialization_failed)
    return dx12_fail(11, "removed device did not report its terminal renderer error");
  if ((*renderer)->last_frame_pixels() || (*renderer)->begin_frame() || (*renderer)->resize({65, 49}, {1}))
    return dx12_fail(12, "removed device retained successful capture or accepted further work");
  frame->reset();
  renderer->reset();
  // This removes an idle native device, not a simulated live-queue wait failure.
  // The independent synchronization fault tests must prove that separate case.
  return dx12_recovery_debug_messages_safe(messages.Get(), true)
      ? 0 : dx12_fail(13, "unsafe removal cleanup");
}

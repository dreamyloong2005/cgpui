#include "../support/frame_wait_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

#include <string>

using namespace cgpui;
using namespace cgpui_test;

namespace {
struct AdapterEnvironment {
  std::string saved;
  AdapterEnvironment() { if (const auto* value = std::getenv("CGPUI_DX12_ADAPTER")) saved = value; }
  ~AdapterEnvironment() { _putenv_s("CGPUI_DX12_ADAPTER", saved.c_str()); }
};
}

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  AdapterEnvironment environment;
  if (!deadline.done || !window.hwnd) return dx12_fail(1, "epoch recovery fixture unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const auto initial = (*renderer)->backend_info();
  const ImageAsset image{.id = {8404}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 255, 0, 255}}};
  std::vector<std::unique_ptr<RenderFrame>> finished;
  for (unsigned epoch = 0; epoch < 4; ++epoch) {
    if (epoch != 0) {
      if (!dx12_remove_native_device((*renderer)->backend_info()))
        return dx12_fail(3, "repeated native removal failed");
      const auto retry = (*renderer)->begin_frame();
      if (retry || retry.error().code != ErrorCode::frame_retry_required)
        return dx12_fail(4, "repeated removal did not recover before retry");
    }
    const auto info = (*renderer)->backend_info();
    if (info.device_generation != initial.device_generation + epoch ||
        info.adapter_luid != initial.adapter_luid || info.software_adapter != initial.software_adapter)
      return dx12_fail(5, "repeated recovery lost generation or pinned adapter");
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(6, "epoch renderer is not ready");
    if (epoch == 0) (*frame)->upload_image(image); // Only one public upload across all generations.
    (*frame)->clear({0, 0, 1, 1});
    (*frame)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(image)});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 255, 0, 255}) ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 40, 40, {0, 0, 255, 255}))
      return dx12_fail(7, "epoch image cache did not survive without reupload");
    finished.push_back(std::move(*frame));
    if (!dx12_recovered_native_device_clean(info)) return dx12_fail(8, "epoch native device debug errors");
    // Changing a construction-only preference must not be re-read on recovery.
    // An invalid value also catches accidental selection followed by fallback.
    if (_putenv_s("CGPUI_DX12_ADAPTER", "not-an-adapter") != 0)
      return dx12_fail(9, "adapter preference mutation failed");
  }
  finished.clear();
  auto final_frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!final_frame) return dx12_fail(10, "old epoch frame destruction poisoned renderer");
  (*final_frame)->clear({0, 1, 0, 1});
  if (!(*final_frame)->request_pixel_capture() || !(*final_frame)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 40, 40, {0, 255, 0, 255}))
    return dx12_fail(11, "final epoch rendering failed after old frame destruction");
  return 0;
}

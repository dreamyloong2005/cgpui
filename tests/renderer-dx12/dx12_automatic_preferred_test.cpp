#include "dx12_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include "../../src/renderer/renderer_backend_selection_internal.hpp"
#include "../../src/renderer/dx12/dx12_initialization_attempt_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
unsigned fallback_calls = 0;
Result<std::unique_ptr<Renderer>> forbidden_fallback(const RenderSurfaceDescriptor&) {
  ++fallback_calls;
  return std::unexpected(Error{ErrorCode::renderer_initialization_failed, "unexpected Vulkan attempt"});
}
}
int main() {
  // Deliberately retain the runner's adapter/debug/GPU-validation environment.
  Dx12Window window;
  if (!window.hwnd) return 1;
  auto renderer = detail::select_renderer_backend(window.descriptor(), "auto",
      forbidden_fallback, create_dx12_renderer, dx12_attempt_renderer_initialization);
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const auto info = (*renderer)->backend_info();
  if (fallback_calls || info.backend != RendererBackend::dx12 ||
      info.selection != RendererBackendSelection::automatic || !info.fallback_reason.empty() ||
      info.adapter_name.empty() || !info.adapter_luid || info.device_generation != 1) return 3;
  auto frame = wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(4, frame.error().message.c_str());
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_rect({.rect = {{8, 8}, {16, 16}}, .color = {1, 0, 0, 1}});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() || fallback_calls ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 12, 12, {255, 0, 0, 255}) ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 40, 40, {0, 0, 255, 255})) return 5;
  const auto current = (*renderer)->backend_info();
  if (current.adapter_luid != info.adapter_luid || current.device_generation != 1 ||
      current.selection != RendererBackendSelection::automatic || !current.fallback_reason.empty()) return 6;
  std::cout << "DX12_AUTO_PREFERRED actual_backend=dx12 selection=automatic luid=" << *info.adapter_luid
            << " software=" << info.software_adapter << " generation=" << info.device_generation
            << " pixels=64x64 adapter=" << info.adapter_name << std::endl;
  return 0;
}

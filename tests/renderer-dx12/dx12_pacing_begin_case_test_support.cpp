#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"
#include <cstdlib>
#include <string_view>

namespace cgpui_test {
int pacing_first_begin_case(bool visible) {
  using namespace cgpui;
  Dx12RecoveryDeadline deadline;
  Dx12Window window; // Actual HWND, hidden by construction; never a test platform mock.
  Dx12PacingForwardApi api; // Must outlive renderer and all outstanding frames below.
  if (!deadline.done || !window.hwnd || IsWindowVisible(window.hwnd) || !pacing_begin_debug_enable())
    return dx12_fail(1, "real hidden HWND/debug setup failed");
  if (visible) ShowWindow(window.hwnd, SW_SHOWNOACTIVATE);
  if ((IsWindowVisible(window.hwnd) != FALSE) != visible)
    return dx12_fail(2, "requested explicit visible/hidden native condition not established");
  auto created = dx12_create_renderer(window.descriptor(), api);
  if (!created) return dx12_fail(3, created.error().message.c_str());
  auto renderer = std::move(*created);
  const auto info = renderer->backend_info();
  const char* adapter = std::getenv("CGPUI_DX12_ADAPTER");
  const bool warp = adapter && std::string_view(adapter) == "warp";
  if (info.backend != RendererBackend::dx12 || info.selection != RendererBackendSelection::dx12 ||
      !info.adapter_luid || !info.fallback_reason.empty() || (warp && !info.software_adapter) ||
      !pacing_begin_debug_clean(info) || api.sets != 1 || api.gets != 1 || api.polls || !api.observed_handle)
    return dx12_fail(4, "real factory did not configure exactly one native latency handle");
  std::cout << "PACING_BEGIN condition=" << (visible ? "visible" : "initially-hidden")
            << " actual=dx12 adapter=" << info.adapter_name << " software=" << info.software_adapter
            << " luid=" << *info.adapter_luid << " sets=" << api.sets << " gets=" << api.gets << '\n';
  auto frame = pacing_begin_bounded(*renderer, api);
  if (!frame || !*frame) return dx12_fail(5, frame ? "null frame returned" : frame.error().message.c_str());
  std::cout << "PACING_BEGIN returned_frame=1 polls=" << api.polls << " ready=" << api.ready
            << " pending=" << api.pending << '\n';
  // R-stage meaningful RED: real Renderer returned a frame without observing admission.
  if (api.polls == 0 || api.ready != 1)
    return dx12_fail(6, "real begin_frame returned before a native WAIT_OBJECT_0 admission");
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_rect({.rect = {{8, 8}, {24, 24}}, .color = {1, 0, 0, 1}});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
      !dx12_pixel(renderer->last_frame_pixels(), 16, 16, {255, 0, 0, 255}) ||
      !dx12_pixel(renderer->last_frame_pixels(), 48, 48, {0, 0, 255, 255}) ||
      !pacing_begin_debug_clean(renderer->backend_info()))
    return dx12_fail(7, "admitted real frame did not present correct independent pixels/debug state");
  frame->reset();
  renderer.reset();
  if (api.closes != 1 || !api.close_ok)
    return dx12_fail(8, "real renderer did not close its app-owned latency handle exactly once");
  return 0;
}
} // namespace cgpui_test

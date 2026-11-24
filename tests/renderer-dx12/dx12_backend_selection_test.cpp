#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

#include <cstdlib>
#include <string>
#include <string_view>

using namespace cgpui;
using namespace cgpui_test;

namespace {
struct Environment {
  const char* name;
  std::string old;
  explicit Environment(const char* variable) : name(variable) {
    if (const char* value = std::getenv(name)) old = value;
  }
  ~Environment() { _putenv_s(name, old.c_str()); }
  void set(const char* value) { _putenv_s(name, value); }
};
}

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "native window failed");
  Environment preference("CGPUI_RENDERER");
  Environment adapter("CGPUI_DX12_ADAPTER");
  preference.set("invalid-backend");
  auto renderer = create_renderer(window.descriptor());
  if (renderer || renderer.error().code != ErrorCode::invalid_argument) {
    return dx12_fail(2, "invalid backend environment ignored");
  }
  adapter.set("not-an-adapter");
  preference.set("dx12");
  renderer = create_renderer(window.descriptor());
  if (renderer) return dx12_fail(3, "explicit DX12 failure fell back");
  preference.set("auto");
  renderer = create_renderer(window.descriptor());
  if (renderer || renderer.error().code != ErrorCode::invalid_argument ||
      renderer.error().message.find("CGPUI_DX12_ADAPTER") == std::string::npos)
    return dx12_fail(4, "automatic selection bypassed an invalid DX12 adapter request");
  preference.set("vulkan");
  renderer = create_renderer(window.descriptor());
  if (!renderer) return dx12_fail(5, renderer.error().message.c_str());
  auto info = (*renderer)->backend_info();
  if (info.backend != RendererBackend::vulkan ||
      info.selection != RendererBackendSelection::vulkan || !info.fallback_reason.empty() ||
      info.adapter_name.empty() || info.device_generation == 0) {
    return dx12_fail(6, "explicit Vulkan was affected by the invalid DX12 adapter request");
  }
  renderer->reset();
  adapter.set("warp");
  for (const char* choice : {"dx12", "auto", ""}) {
    Dx12Window fresh;
    if (!fresh.hwnd) return dx12_fail(7, "fresh selection HWND unavailable");
    preference.set(choice);
    auto selected = create_renderer(fresh.descriptor());
    if (!selected) return dx12_fail(8, selected.error().message.c_str());
    info = (*selected)->backend_info();
    const auto selection = std::string_view(choice) == "dx12"
        ? RendererBackendSelection::dx12 : RendererBackendSelection::automatic;
    if (info.backend != RendererBackend::dx12 || info.selection != selection ||
        !info.software_adapter || info.adapter_name.empty() || !info.adapter_luid ||
        !info.device_generation || !info.fallback_reason.empty())
      return dx12_fail(9, "explicit or healthy automatic WARP backend metadata differs");
    auto frame = cgpui_test::wait_for_test_frame(**selected);
    if (!frame) return dx12_fail(10, frame.error().message.c_str());
    (*frame)->clear({1, 0, 0, 1});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !dx12_pixel((*selected)->last_frame_pixels(), 5, 5, {255, 0, 0, 255}))
      return dx12_fail(11, "dispatcher did not forward actual capture");
  }
  return 0;
}

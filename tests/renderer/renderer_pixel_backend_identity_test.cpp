#include "renderer_pixel_test_support.hpp"

#include <iostream>

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool identity_contract() {
  for (const auto selector : {"", "auto", "vulkan"}) {
    const auto parsed = parse_pixel_test_backend(selector);
    if (!parsed || *parsed != RendererBackend::vulkan) return false;
  }
  const auto dx12 = parse_pixel_test_backend("dx12");
  if (!dx12 || *dx12 != RendererBackend::dx12) return false;
  for (const auto selector : {"both", "DX12", "dx12 ", "metal", "invalid"}) {
    const auto parsed = parse_pixel_test_backend(selector);
    if (parsed || parsed.error().code != ErrorCode::invalid_argument) return false;
  }
  for (const auto expected : {RendererBackend::vulkan, RendererBackend::dx12}) {
    RendererBackendInfo info{.backend = expected,
        .selection = expected == RendererBackend::dx12
            ? RendererBackendSelection::dx12 : RendererBackendSelection::vulkan};
    if (!validate_pixel_backend_info(expected, info)) return false;
    auto wrong = info;
    wrong.backend = RendererBackend::unknown;
    if (validate_pixel_backend_info(expected, wrong)) return false;
    wrong = info;
    wrong.backend = expected == RendererBackend::dx12 ? RendererBackend::vulkan : RendererBackend::dx12;
    if (validate_pixel_backend_info(expected, wrong)) return false;
    wrong = info;
    wrong.selection = RendererBackendSelection::automatic;
    if (validate_pixel_backend_info(expected, wrong)) return false;
    wrong = info;
    wrong.fallback_reason = "unexpected fallback";
    if (validate_pixel_backend_info(expected, wrong)) return false;
    if (validate_pixel_backend_info(expected, info, true)) return false;
    info.software_adapter = true;
    if (!validate_pixel_backend_info(expected, info, true)) return false;
  }
  return !validate_pixel_backend_info(RendererBackend::unknown, {});
}
} // namespace

int main() {
  if (!identity_contract()) {
    std::cerr << "pixel backend selector/identity contract failed\n";
    return 1;
  }
  RendererPixelTestWindow window(L"CGPUIRendererPixelIdentity", L"Selected backend identity");
  // This is the same real factory/capture route used by the 13 scenes, not a stub.
  const auto pixels = capture_pixels(window, {0, 0, 1, 1}, [](RenderFrame& frame) {
    frame.draw_rect({.rect = {{8, 8}, {16, 16}}, .color = {1, 0, 0, 1}});
  });
  return pixels && pixel_near(*pixels, 12, 12, {255, 0, 0, 255}) &&
         pixel_near(*pixels, 40, 40, {0, 0, 255, 255}) ? 0 : 2;
}

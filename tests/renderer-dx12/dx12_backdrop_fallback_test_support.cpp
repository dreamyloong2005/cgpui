#include "dx12_backdrop_fallback_test_support.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_recovery_internal.hpp"
#include <cstdlib>
#include <string_view>

namespace cgpui_test {
bool Dx12BackdropFallbackFixture::initialize(float scale) {
  using namespace cgpui;
  state = std::make_shared<Dx12State>();
  auto descriptor = window.descriptor();
  descriptor.framebuffer_size = {64 * scale, 64 * scale};
  descriptor.scale = {scale};
  if (!window.hwnd || !dx12_configure_surface(*state, descriptor) ||
      !dx12_create_device(*state) || !dx12_create_pipeline(*state) ||
      !dx12_create_swapchain(*state) || !dx12_check_debug_errors(*state)) return false;
  actual_support = dx12_backdrop_blur_support(*state->device.Get());
  state->backdrop_blur->support = actual_support;
  const auto& info = state->info;
  const char* requested = std::getenv("CGPUI_DX12_ADAPTER");
  const bool warp = requested && std::string_view(requested) == "warp";
  std::cout << "BACKDROP_FALLBACK requested=dx12 actual_backend="
            << (info.backend == RendererBackend::dx12 ? "dx12" : "unexpected")
            << " adapter=" << info.adapter_name << " software=" << info.software_adapter
            << " vendor=" << info.vendor_id << " luid=" << info.adapter_luid.value_or(0)
            << " generation=" << info.device_generation << " scale=" << scale
            << " actual_blur_support=" << actual_support.supported << '\n';
  return info.backend == RendererBackend::dx12 && info.selection == RendererBackendSelection::dx12 &&
      info.fallback_reason.empty() && info.adapter_luid &&
      (!warp || info.software_adapter);
}
void Dx12BackdropFallbackFixture::force_unsupported() {
  state->backdrop_blur->support = {.supported = false,
      .reason = "test-forced unsupported backdrop"};
}
std::unique_ptr<cgpui::Dx12Frame> Dx12BackdropFallbackFixture::begin() {
  if (auto ready = cgpui::dx12_prepare_frame(*state); !ready) {
    std::cerr << "fallback fixture begin: " << ready.error().message << '\n';
    return {};
  }
  auto frame = std::make_unique<cgpui::Dx12Frame>(state);
  state->frame_active = true;
  return frame;
}
bool Dx12BackdropFallbackFixture::capture(cgpui::Dx12Frame& frame) {
  if (!frame.request_pixel_capture()) return false;
  if (auto result = frame.present(); !result) {
    std::cerr << "fallback fixture present error=" << static_cast<int>(result.error().code)
              << ": " << result.error().message << '\n';
    return false;
  }
  return state->pixels && state->pixels->valid() &&
      state->pixels->width == state->width && state->pixels->height == state->height;
}
bool Dx12BackdropFallbackFixture::pixel(float x, float y,
    std::array<unsigned char, 4> expected) const {
  return dx12_pixel(state->pixels ? &*state->pixels : nullptr,
      static_cast<unsigned>(x * state->scale.value),
      static_cast<unsigned>(y * state->scale.value), expected);
}
cgpui::BackdropBlurDraw fallback_panel() {
  return {.rect = {{8, 8}, {48, 48}}, .radius = cgpui::BorderRadii::all(8),
      .blur_radius = 4, .tint = cgpui::Color{1, 0, 0, 0.5F}};
}
void fallback_split_scene(cgpui::RenderFrame& frame) {
  frame.clear({1, 1, 1, 1});
  frame.draw_rect({.rect = {{0, 0}, {32, 64}}, .color = {0, 0, 0, 1}});
}
void fallback_foreground(cgpui::RenderFrame& frame) {
  frame.draw_rect({.rect = {{30, 28}, {4, 8}}, .color = {0, 1, 0, 1}});
}
} // namespace cgpui_test

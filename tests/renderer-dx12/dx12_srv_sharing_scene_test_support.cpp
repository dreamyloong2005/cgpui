#include "dx12_srv_topology_test_support.hpp"
#include "../../src/renderer/dx12/dx12_backdrop_blur_internal.hpp"

namespace cgpui_test {
namespace {
constexpr std::array<std::array<unsigned char, 4>, 8> colors{{
    {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 0, 255},
    {255, 0, 255, 255}, {0, 255, 255, 255}, {255, 255, 255, 255}, {64, 128, 192, 255}}};
}
std::shared_ptr<cgpui::Dx12State> srv_native_state(Dx12Window* window, bool alpha) {
  using namespace cgpui;
  auto state = std::make_shared<Dx12State>();
  if (window) {
    auto descriptor = window->descriptor();
    descriptor.transparent_background = alpha;
    if (!window->hwnd || !dx12_configure_surface(*state, descriptor)) return {};
  }
  if (!dx12_create_device(*state)) return {};
  if (state->info.backend != RendererBackend::dx12 || state->info.selection != RendererBackendSelection::dx12 ||
      !state->info.fallback_reason.empty()) return {};
  std::cout << "SRV native backend=dx12 adapter=" << state->info.adapter_name
            << " software=" << state->info.software_adapter << " luid=" << state->info.adapter_luid.value_or(0)
            << " generation=" << state->info.device_generation << '\n';
  state->backdrop_blur->support = dx12_backdrop_blur_support(*state->device.Get());
  if (window && (!dx12_create_pipeline(*state) || !dx12_create_swapchain(*state))) return {};
  return state;
}
cgpui::ImageAsset srv_tile_image(unsigned index) {
  const auto& color = colors.at(index);
  return {.id = {9900 + index}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4,
                 .pixels = {color[0], color[1], color[2], color[3]}}};
}
bool srv_render_tiles(const std::shared_ptr<cgpui::Dx12State>& state, bool upload, bool blur,
                      std::optional<std::size_t> expected_upload) {
  using namespace cgpui;
  state->frame_active = true;
  Dx12Frame frame(state);
  frame.clear({0, 0, 0, 1});
  // All immutable IDs uploaded before draws; unaffected by image-batch policy.
  if (upload) for (unsigned index = 0; index != 8; ++index) frame.upload_image(srv_tile_image(index));
  for (unsigned index = 0; index != 8; ++index)
    frame.draw_image({.bounds = {{float(index % 4 * 16), float(index / 4 * 16)}, {16, 16}},
        .asset = describe_image_asset(srv_tile_image(index)), .sampling = ImageSamplingMode::nearest});
  if (blur) {
    frame.draw_backdrop_blur({.rect = {{0, 40}, {16, 16}}, .blur_radius = 0, .tint = Color{0, 0, 1, 1}});
    frame.draw_backdrop_blur({.rect = {{32, 40}, {16, 16}}, .blur_radius = 0, .tint = Color{0, 1, 0, 1}});
  }
  if (!frame.request_pixel_capture()) return false;
  const auto presented = frame.present(); // Raw failure, no fallback renderer.
  if (!presented) { std::cerr << presented.error().message << '\n'; return false; }
  if (state->info.backend != RendererBackend::dx12 || !state->pixels || !state->diagnostics) return false;
  for (unsigned index = 0; index != 8; ++index)
    for (unsigned y = 0; y != 16; ++y) for (unsigned x = 0; x != 16; ++x)
      if (!dx12_pixel(&*state->pixels, index % 4 * 16 + x, index / 4 * 16 + y, colors[index], 0)) return false;
  if (blur && (!dx12_pixel(&*state->pixels, 8, 48, {0, 0, 255, 255}, 0) ||
               !dx12_pixel(&*state->pixels, 40, 48, {0, 255, 0, 255}, 0))) return false;
  const auto draws = blur ? 10U : 8U;
  const auto& diagnostic = *state->diagnostics;
  if (!diagnostic_work(&diagnostic, draws, draws, draws) || diagnostic.submitted_draws.image_count != 8 ||
      diagnostic.upload_bytes.image_byte_count != expected_upload.value_or(upload ? 32U : 0U) ||
      diagnostic.submitted_draws.backdrop_blur_count != (blur ? 2U : 0U) ||
      diagnostic.backdrop_blur.blurred_backdrop_count != (blur ? 2U : 0U)) return false;
  std::cout << "DX12 raw Present/capture, exact eight tiles and planned/submitted counts passed\n";
  return true;
}
int srv_sharing_case(bool blur) {
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state || !srv_render_tiles(state, true, blur)) return dx12_fail(1, "SRV scene/pixel/diagnostic precondition failed");
  return srv_topology(*state, blur ? 14U : 8U, 1)
      ? 0 : dx12_fail(2, "live SRV views must share one native heap AFTER pixel success");
}
} // namespace cgpui_test

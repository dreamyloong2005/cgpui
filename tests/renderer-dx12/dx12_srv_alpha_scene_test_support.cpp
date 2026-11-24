#include "dx12_srv_alpha_scene_test_support.hpp"
#include "dx12_srv_pool_test_support.hpp"
#include "dx12_popup_test_support.hpp"
#include "../../src/renderer/dx12/dx12_backdrop_blur_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"

namespace cgpui_test {
int srv_alpha_case(unsigned capacity) {
  using namespace cgpui;
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready()) return dx12_fail(1, "SRV alpha desktop fixture unavailable");
  CompositionTestWindows windows(48, 48);
  if (!windows.valid() || !windows.expect({0, 255, 0}, windows.backdrop_point()))
    return dx12_fail(2, "SRV alpha controlled desktop not visible");
  auto state = std::make_shared<Dx12State>();
  state->srv_pool = std::make_unique<Dx12SrvDescriptorPool>(capacity);
  if (!dx12_configure_surface(*state, dx12_popup_descriptor(windows.popup())) || !dx12_create_device(*state) ||
      !dx12_create_pipeline(*state) || !dx12_create_swapchain(*state)) return dx12_fail(3, "SRV alpha native setup failed");
  state->backdrop_blur->support = dx12_backdrop_blur_support(*state->device.Get());
  if (!state->composition_surface || !state->backdrop_blur->support.supported)
    return dx12_fail(4, "real alpha blur unsupported in fixture");
  state->frame_active = true;
  Dx12Frame frame(state);
  frame.clear({0, 0, 0, 0});
  for (unsigned index = 0; index != 3; ++index) frame.upload_image(srv_tile_image(index));
  for (unsigned index = 0; index != 3; ++index)
    frame.draw_image({.bounds = {{float(index * 10), 0}, {8, 8}},
        .asset = describe_image_asset(srv_tile_image(index)), .sampling = ImageSamplingMode::nearest});
  frame.draw_rect({.rect = {{0, 32}, {64, 32}}, .color = {1, 0, 0, 0.5F}});
  frame.draw_backdrop_blur({.rect = {{0, 32}, {16, 16}}, .blur_radius = 0,
      .tint = Color{0, 0, 1, 0.5F}, .metadata = {.opacity = 0.5F}});
  // At capacity4, blur3 must skip the image page's single free slot. The first
  // glyph uses that hole; a newly published glyph snapshot then fills page2.
  frame.draw_text(diagnostic_text("M", 16, {32, 0}));
  frame.draw_image({.bounds = {{52, 0}, {8, 8}}, .asset = describe_image_asset(srv_tile_image(0)),
      .sampling = ImageSamplingMode::linear});
  frame.draw_text(diagnostic_text("W", 16, {32, 16}));
  frame.draw_backdrop_blur({.rect = {{32, 32}, {16, 16}}, .blur_radius = 0,
      .tint = Color{0, 0, 1, 0.5F}, .metadata = {.opacity = 0.5F}});
  if (!frame.request_pixel_capture()) return dx12_fail(5, "SRV alpha capture unavailable");
  const auto presented = frame.present();
  if (!presented) return dx12_fail(6, presented.error().message.c_str());
  const auto* pixels = state->pixels ? &*state->pixels : nullptr;
  if (!dx12_pixel(pixels, 4, 4, {255, 0, 0, 255}, 0) || !dx12_pixel(pixels, 14, 4, {0, 255, 0, 255}, 0) ||
      !dx12_pixel(pixels, 24, 4, {0, 0, 255, 255}, 0) || !dx12_pixel(pixels, 56, 4, {255, 0, 0, 255}, 0) ||
      !dx12_pixel(pixels, 8, 40, {96, 0, 64, 160}, 2) || !dx12_pixel(pixels, 40, 40, {96, 0, 64, 160}, 2) ||
      !dx12_pixel(pixels, 24, 40, {128, 0, 0, 128}, 1) || !dx12_pixel(pixels, 4, 20, {0, 0, 0, 0}, 0) ||
      !diagnostic_ink(pixels, 32, 0, 50, 18) || !diagnostic_ink(pixels, 32, 16, 50, 34) ||
      !windows.expect({96, 96, 64}, windows.popup_point(8, 40)) ||
      !windows.expect({0, 255, 0}, windows.popup_point(4, 20)))
    return dx12_fail(7, "SRV mixed bindings, premultiplied alpha or desktop composite changed");
  unsigned glyphs = 0;
  for (const auto& [source, native] : state->texture_renderer->native_textures)
    glyphs += source->format == DXGI_FORMAT_R8_UNORM;
  if (glyphs != 2 || !state->diagnostics || !diagnostic_work(&*state->diagnostics, 9, 9, 9) ||
      state->diagnostics->submitted_draws.image_count != 4 || state->diagnostics->submitted_draws.text_count != 2 ||
      state->diagnostics->backdrop_blur.blurred_backdrop_count != 2 || !state->diagnostics->upload_bytes.glyph_atlas_byte_count)
    return dx12_fail(8, "two immutable glyph snapshots/mixed draw accounting missing");
  const auto pages = capacity == 4 ? 3U : 1U;
  if (!srv_topology(*state, 11, pages) || state->srv_pool->stats().successful_page_creations != pages ||
      state->srv_pool->stats().live_slots != 11 || state->backdrop_blur->frame_targets.size() != 2)
    return dx12_fail(9, "mixed image/glyph/blur actual native topology wrong");
  for (const auto& group : state->backdrop_blur->frame_targets)
    for (unsigned index = 0; index != 3; ++index)
      if (group->srvs.size() != 3 || group->targets[index].srv.ptr != group->srvs.gpu(index).ptr)
        return dx12_fail(10, "alpha original/filtered SRVs are not the same atomic page group");
  return dx12_check_debug_errors(*state) ? 0 : dx12_fail(11, "mixed SRV alpha GPU validation error");
}
} // namespace cgpui_test

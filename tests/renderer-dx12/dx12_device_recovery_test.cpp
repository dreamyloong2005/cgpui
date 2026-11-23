#include "../support/frame_wait_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
TextDraw recovery_text() {
  const FontDescriptor font{.family = "Segoe UI"};
  const auto shaped = shape_text("M", font, 16, {1},
      {.preferred_backend = TextShapingBackend::deterministic_fallback});
  return {.bounds = {{36, 4}, {24, 24}}, .color = {0, 0, 1, 1}, .font = font,
      .content = "M", .byte_length = 1, .font_size = 16, .scale = {1},
      .device_font_size = 16, .glyphs = text_glyph_paint_metadata(shaped, {36, 4})};
}

void paint(RenderFrame& frame, const ImageAssetDescriptor& image, const TextDraw& text) {
  frame.clear({1, 1, 1, 1});
  frame.draw_image({.bounds = {{4, 4}, {16, 16}}, .asset = image});
  frame.draw_text(text);
  frame.draw_box_shadow({.rect = {{4, 24}, {8, 4}},
      .shadow = {.color = {0, 0, 0, 1}, .offset = {12, 0}, .blur_radius = 0}});
  frame.draw_rect({.rect = {{0, 32}, {32, 32}}, .color = {0, 0, 0, 1}});
  frame.draw_backdrop_blur({.rect = {{8, 36}, {48, 24}}, .blur_radius = 4});
}

bool scene_visible(const RendererFramePixels* pixels, std::array<unsigned char, 4> image) {
  if (!pixels || !pixels->valid() || pixels->width != 64 || pixels->height != 64 ||
      !dx12_pixel(pixels, 8, 8, image) ||
      !dx12_pixel(pixels, 18, 26, {0, 0, 0, 255}) ||
      !dx12_pixel(pixels, 6, 26, {255, 255, 255, 255})) return false;
  const auto left = pixels->pixel_rgba8(28, 48), right = pixels->pixel_rgba8(35, 48);
  if (!left || !right || (*left)[3] != 255 || (*right)[3] != 255) return false;
  for (unsigned channel = 0; channel < 3; ++channel) {
    if ((*left)[channel] < 24 || (*left)[channel] > 96 ||
        (*right)[channel] < 159 || (*right)[channel] > 231) return false;
  }
  unsigned ink = 0;
  for (unsigned y = 4; y < 28; ++y) {
    for (unsigned x = 36; x < 60; ++x) {
      const auto pixel = pixels->pixel_rgba8(x, y);
      if (pixel && (*pixel)[0] < 200 && (*pixel)[1] < 200 && (*pixel)[2] > 250) ++ink;
    }
  }
  return ink > 8; // Prove a real glyph was rendered, not just an equal blank capture.
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done) return dx12_fail(1, "recovery watchdog unavailable");
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(2, "recovery window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(3, renderer.error().message.c_str());
  const auto before = (*renderer)->backend_info();
  const ImageAsset old_image{.id = {8401}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  auto current_image = old_image;
  current_image.bitmap.pixels = {0, 255, 0, 255};
  const auto asset = describe_image_asset(old_image);
  const auto text = recovery_text();
  auto old_frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!old_frame) return dx12_fail(4, "old snapshot warmup begin failed");
  (*old_frame)->upload_image(old_image);
  paint(**old_frame, asset, text);
  if (!(*old_frame)->request_pixel_capture() || !(*old_frame)->present() ||
      !scene_visible((*renderer)->last_frame_pixels(), {255, 0, 0, 255}))
    return dx12_fail(5, "old image/text/effect warmup failed");
  const auto* initial_diagnostics = (*renderer)->last_frame_diagnostic_snapshot();
  if (!initial_diagnostics || initial_diagnostics->upload_bytes.image_byte_count != 4 ||
      !initial_diagnostics->upload_bytes.glyph_atlas_byte_count)
    return dx12_fail(15, "initial diagnostic texture uploads absent");
  const auto glyph_bytes = initial_diagnostics->upload_bytes.glyph_atlas_byte_count;
  auto current_frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!current_frame) return dx12_fail(6, "replacement snapshot warmup begin failed");
  (*current_frame)->invalidate_image(asset.id);
  (*current_frame)->upload_image(current_image);
  paint(**current_frame, asset, text);
  if (!(*current_frame)->request_pixel_capture() || !(*current_frame)->present() ||
      !scene_visible((*renderer)->last_frame_pixels(), {0, 255, 0, 255}))
    return dx12_fail(7, "replacement image/text/effect warmup failed");
  const auto* replacement_diagnostics = (*renderer)->last_frame_diagnostic_snapshot();
  if (!replacement_diagnostics || replacement_diagnostics->upload_bytes.image_byte_count != 4 ||
      replacement_diagnostics->upload_bytes.glyph_atlas_byte_count)
    return dx12_fail(16, "replacement image or cached glyph diagnostic uploads differ");
  const auto expected = (*renderer)->last_frame_pixels()->rgba8;
  // Keep both finished frames alive: the invalidated red snapshot is no longer
  // reachable through the current image cache, but must not pin the old device.
  if (!dx12_remove_native_device(before)) return dx12_fail(8, "native removal was not verified");
  const auto interrupted = (*renderer)->begin_frame();
  if (interrupted || interrupted.error().code != ErrorCode::frame_retry_required)
    return dx12_fail(9, "removed device did not complete recovery before requesting retry");
  const auto after = (*renderer)->backend_info();
  if (after.adapter_luid != before.adapter_luid ||
      after.device_generation != before.device_generation + 1 || (*renderer)->last_frame_pixels())
    return dx12_fail(10, "recovery adapter, generation or stale capture differs");
  auto recovered_frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!recovered_frame) return dx12_fail(11, "recovered renderer is not immediately usable");
  paint(**recovered_frame, asset, text); // No image reupload after loss.
  if (!(*recovered_frame)->request_pixel_capture() || !(*recovered_frame)->present() ||
      !scene_visible((*renderer)->last_frame_pixels(), {0, 255, 0, 255}))
    return dx12_fail(12, "recovered cached image/text/effect rendering failed");
  const auto* recovered_diagnostics = (*renderer)->last_frame_diagnostic_snapshot();
  if (!recovered_diagnostics || recovered_diagnostics->upload_bytes.image_byte_count != 4 ||
      recovered_diagnostics->upload_bytes.glyph_atlas_byte_count != glyph_bytes ||
      !recovered_diagnostics->work.exact_match())
    return dx12_fail(17, "diagnostics did not count actual image/glyph recovery reuploads");
  if ((*renderer)->last_frame_pixels()->rgba8 != expected)
    return dx12_fail(13, "recovered scene differs from the validated pre-loss scene");
  if ((*old_frame)->present() || (*current_frame)->present())
    return dx12_fail(14, "finished pre-loss frames became presentable after recovery");
  return 0;
}

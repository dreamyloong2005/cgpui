#include "../support/frame_wait_test_support.hpp"
#include "dx12_diagnostics_text_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, "upload diagnostics renderer unavailable");
  const auto image = diagnostic_image();
  const auto asset = describe_image_asset(image);
  for (unsigned pass = 0; pass != 2; ++pass) {
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(2, "upload diagnostics image frame unavailable");
    (*frame)->clear({0, 0, 0, 1});
    if (pass == 0) (*frame)->upload_image(image);
    for (const float x : {0.0F, 24.0F})
      (*frame)->draw_image({.bounds = {{x, 0}, {16, 16}}, .asset = asset, .sampling = ImageSamplingMode::nearest});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 4, 4, {255, 0, 0, 255}) ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 36, 12, {255, 255, 255, 255}))
      return dx12_fail(3, "repeated/cached image diagnostic pixels differ");
    const auto* snapshot = (*renderer)->last_frame_diagnostic_snapshot();
    if (!diagnostic_work(snapshot, 2, 2, 2) || snapshot->submitted_draws.image_count != 2 ||
        snapshot->upload_bytes.image_byte_count != (pass == 0 ? 16U : 0U) || snapshot->upload_bytes.glyph_atlas_byte_count)
      return dx12_fail(4, "image bytes counted row padding, duplicate draws, or cached uploads");
  }

  GlyphCache reference;
  const auto text = diagnostic_text("MM", 16, {4, 4});
  const auto quads = build_textured_glyph_quads(text, reference);
  const auto lower = diagnostic_text("MM", 16, {4, 32});
  const auto lower_quads = build_textured_glyph_quads(lower, reference);
  if (quads.size() != 2 || lower_quads.size() != 2 || reference.upload_records().size() != 1 ||
      diagnostic_atlas_runs(quads) != 1) return dx12_fail(5, "text run/upload oracle unavailable");
  for (unsigned pass = 0; pass != 2; ++pass) {
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(6, "upload diagnostics text frame unavailable");
    (*frame)->clear({0, 0, 0, 1});
    (*frame)->draw_text(text);
    if (pass != 0) (*frame)->draw_text(lower);
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !diagnostic_glyph_pixels((*renderer)->last_frame_pixels(), quads, reference) ||
        (pass != 0 && !diagnostic_glyph_pixels((*renderer)->last_frame_pixels(), lower_quads, reference)))
      return dx12_fail(7, "text diagnostic coverage differs");
    const auto* snapshot = (*renderer)->last_frame_diagnostic_snapshot();
    const std::size_t count = pass + 1;
    if (!diagnostic_work(snapshot, count, count, count) || snapshot->submitted_draws.text_count != count ||
        snapshot->upload_bytes.image_byte_count ||
        snapshot->upload_bytes.glyph_atlas_byte_count != (pass == 0 ? diagnostic_atlas_bytes(reference) : 0))
      return dx12_fail(8, "text counts confused glyphs with atlas runs, or re-counted cached texture bytes");
  }
  return 0;
}

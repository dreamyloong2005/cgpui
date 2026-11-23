#include "dx12_diagnostics_text_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer || !(*renderer)->resize({840, 840}, {1})) return dx12_fail(1, "atlas run diagnostics renderer unavailable");
  auto text = diagnostic_text("MBWM", 400, {});
  if (text.glyphs.size() != 4) return dx12_fail(2, "multi-atlas diagnostics shaping failed");
  for (std::size_t index = 0; index != text.glyphs.size(); ++index)
    text.glyphs[index].device_origin = {4.0F + float(index % 2) * 420, 4.0F + float(index / 2) * 420};
  GlyphCache reference;
  const auto quads = build_textured_glyph_quads(text, reference);
  const auto runs = diagnostic_atlas_runs(quads);
  if (quads.size() != 4 || reference.atlas_pages().size() < 3 || runs < 3 ||
      quads.front().page_index != quads.back().page_index)
    return dx12_fail(3, "text diagnostics did not exercise multiple pages and return to an earlier page");
  for (unsigned pass = 0; pass != 2; ++pass) {
    auto frame = (*renderer)->begin_frame();
    if (!frame) return dx12_fail(4, "atlas run diagnostics frame unavailable");
    (*frame)->clear({0, 0, 0, 1});
    (*frame)->draw_text(text);
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !diagnostic_glyph_pixels((*renderer)->last_frame_pixels(), quads, reference))
      return dx12_fail(5, "multi-atlas diagnostic frame failed to render real glyphs");
    const auto* snapshot = (*renderer)->last_frame_diagnostic_snapshot();
    if (!diagnostic_work(snapshot, 1, 1, runs) || snapshot->submitted_draws.text_count != runs ||
        snapshot->upload_bytes.image_byte_count ||
        snapshot->upload_bytes.glyph_atlas_byte_count != (pass == 0 ? diagnostic_atlas_bytes(reference) : 0))
      return dx12_fail(6, "atlas runs inflated original command count or duplicate/cached page upload bytes");
  }
  return 0;
}

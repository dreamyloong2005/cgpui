#pragma once

#include "dx12_test_support.hpp"
#include <tuple>

namespace cgpui_test {
inline auto diagnostic_counts(const cgpui::RendererDrawCounts& v) {
  return std::make_tuple(v.solid_rect_count, v.rounded_rect_count, v.text_count, v.text_selection_count,
      v.text_caret_count, v.image_count, v.box_shadow_count, v.backdrop_blur_count, v.total_count, v.saturated);
}
inline auto diagnostic_uploads(const cgpui::RendererUploadByteCounts& v) {
  return std::make_tuple(v.glyph_atlas_byte_count, v.image_byte_count, v.total_byte_count, v.saturated);
}
inline auto diagnostic_timings(const cgpui::RendererFrameTimings& v) {
  return std::make_tuple(v.pacing_wait_nanoseconds, v.resource_preparation_nanoseconds,
      v.image_acquisition_nanoseconds, v.command_recording_nanoseconds, v.queue_submission_nanoseconds,
      v.presentation_nanoseconds, v.total_nanoseconds, v.saturated);
}
inline auto diagnostic_pending(const cgpui::RendererFrameDiagnostics& v) {
  return std::make_tuple(v.pending_batch_count, v.unexpected_batch_count, v.pending_command_count,
      v.unexpected_command_count, v.pending_upload_byte_count, v.unexpected_upload_byte_count,
      v.pending_draw_count, v.unexpected_draw_count);
}
inline bool diagnostic_same(const cgpui::RendererFrameDiagnosticSnapshot* a,
                            const cgpui::RendererFrameDiagnosticSnapshot& b) {
  return a && a->work.planned_work == b.work.planned_work && a->work.submitted_work == b.work.submitted_work &&
      diagnostic_pending(a->work) == diagnostic_pending(b.work) &&
      diagnostic_uploads(a->upload_bytes) == diagnostic_uploads(b.upload_bytes) &&
      diagnostic_counts(a->planned_draws) == diagnostic_counts(b.planned_draws) &&
      diagnostic_counts(a->submitted_draws) == diagnostic_counts(b.submitted_draws) &&
      a->dropped_resources.resources == b.dropped_resources.resources &&
      diagnostic_counts(a->dropped_resources.counts) == diagnostic_counts(b.dropped_resources.counts) &&
      a->dropped_resources.unsupported_primitive_count == b.dropped_resources.unsupported_primitive_count &&
      a->dropped_resources.missing_submission_resource_count == b.dropped_resources.missing_submission_resource_count &&
      diagnostic_timings(a->timings) == diagnostic_timings(b.timings) &&
      a->backdrop_blur.blurred_backdrop_count == b.backdrop_blur.blurred_backdrop_count &&
      a->backdrop_blur.offscreen_pass_count == b.backdrop_blur.offscreen_pass_count &&
      a->backdrop_blur.offscreen_targets_allocated == b.backdrop_blur.offscreen_targets_allocated;
}
inline bool diagnostic_work(const cgpui::RendererFrameDiagnosticSnapshot* v,
                            std::size_t planned, std::size_t submitted, std::size_t draws) {
  if (!v || planned < submitted) return false;
  const auto& t = v->timings;
  const auto elapsed = t.pacing_wait_nanoseconds + t.resource_preparation_nanoseconds +
      t.image_acquisition_nanoseconds + t.command_recording_nanoseconds + t.queue_submission_nanoseconds +
      t.presentation_nanoseconds;
  return v->work.planned_work.command_count == planned && v->work.submitted_work.command_count == submitted &&
      v->work.planned_work.draw_count == draws && v->work.submitted_work.draw_count == draws &&
      v->planned_draws.total_count == draws && v->submitted_draws.total_count == draws &&
      diagnostic_counts(v->planned_draws) == diagnostic_counts(v->submitted_draws) &&
      !v->planned_draws.saturated && v->dropped_resources.empty() &&
      v->work.pending_command_count == planned - submitted && v->work.unexpected_command_count == 0 &&
      v->work.pending_draw_count == 0 && v->work.unexpected_draw_count == 0 &&
      v->upload_bytes.total_byte_count == v->upload_bytes.image_byte_count + v->upload_bytes.glyph_atlas_byte_count &&
      v->work.planned_work.upload_byte_count == v->upload_bytes.total_byte_count &&
      v->work.submitted_work.upload_byte_count == v->upload_bytes.total_byte_count &&
      !v->upload_bytes.saturated && !t.saturated && elapsed == t.total_nanoseconds && elapsed > 0;
}
inline cgpui::ImageAsset diagnostic_image() {
  // Four RGBA pixels: padding between rows must not become upload payload.
  return {.id = {9701}, .logical_size = {2, 2}, .bitmap = {.width = 2, .height = 2, .stride = 12,
      .pixels = {255, 0, 0, 255, 0, 255, 0, 255, 7, 19, 31, 43,
                 0, 0, 255, 255, 255, 255, 255, 255}}};
}
inline cgpui::TextDraw diagnostic_text(const std::string& content, float size, cgpui::Point origin) {
  const cgpui::FontDescriptor font{.family = "Segoe UI"};
  const auto shaped = cgpui::shape_text(content, font, size, {1},
      {.preferred_backend = cgpui::TextShapingBackend::deterministic_fallback});
  return {.bounds = {origin, {1000, size}}, .color = {1, 1, 1, 1}, .font = font,
      .content = content, .byte_length = content.size(), .font_size = size, .scale = {1},
      .device_font_size = size, .glyphs = cgpui::text_glyph_paint_metadata(shaped, origin)};
}
inline bool diagnostic_ink(const cgpui::RendererFramePixels* pixels, unsigned x0, unsigned y0,
                           unsigned x1, unsigned y1) {
  if (!pixels || !pixels->valid()) return false;
  unsigned ink = 0;
  for (unsigned y = y0; y < y1; ++y) for (unsigned x = x0; x < x1; ++x) {
    const auto pixel = pixels->pixel_rgba8(x, y);
    if (pixel && (*pixel)[0] > 16 && (*pixel)[0] == (*pixel)[1] && (*pixel)[1] == (*pixel)[2]) ++ink;
  }
  return ink > 8;
}
} // namespace cgpui_test

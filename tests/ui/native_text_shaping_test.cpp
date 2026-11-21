#include "cgpui/ui/text.hpp"
#include "cgpui/ui/paint.hpp"
#include "native_text_shaping_reference.hpp"
#include "text_selection_paint_geometry.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}

bool near(float a, float b, float tolerance) {
  return std::fabs(a - b) <= tolerance;
}

#if defined(_WIN32)
const cgpui::FontDescriptor proportional{.family = "Segoe UI"};
const cgpui::FontDescriptor ligature_font{.family = "Calibri"};
#elif defined(__APPLE__)
const cgpui::FontDescriptor proportional{.family = "Helvetica Neue"};
const cgpui::FontDescriptor ligature_font{.family = "Helvetica Neue"};
#else
const cgpui::FontDescriptor proportional{.family = "DejaVu Sans"};
const cgpui::FontDescriptor ligature_font{.family = "DejaVu Sans"};
#endif

float column_x(const cgpui::TextMeasurement& measurement, std::size_t byte) {
  float x = 0.0F;
  for (const auto& column : measurement.grapheme_columns) {
    if (column.byte_start >= byte) break;
    x += column.advance;
  }
  return x;
}

void latin_proportional_advances() {
  const auto run = cgpui::shape_text("Floating panel", proportional, 28.0F);
  require(run.used_backend != cgpui::TextShapingBackend::deterministic_fallback,
          "default shaping did not use the native backend");
  require(run.glyphs.size() == 14, "Latin text should shape one glyph per letter");
  const float l = run.glyphs[1].advance;
  const float o = run.glyphs[2].advance;
  const float i = run.glyphs[5].advance;
  require(l < o && i < o, "narrow glyphs (l, i) must advance less than o");
  const float reference = native_text_test::reference_layout_width(run);
  require(reference > 0.0F && near(run.total_advance, reference, 0.5F),
          "shaped width differs from the platform layout width by more than 0.5 px: " +
              std::to_string(run.total_advance) + " vs " + std::to_string(reference));
  std::cout << "Floating panel @28: shaped " << run.total_advance
            << " px, platform layout " << reference << " px\n";
}

void font_metrics_are_real() {
  const auto run = cgpui::shape_text("Hx", proportional, 20.0F);
  const cgpui::TextFontMetrics& m = run.font_metrics;
  require(m.ascent > 0.0F && m.descent > 0.0F, "ascent/descent missing");
  require(m.x_height > 0.0F && m.x_height < m.cap_height && m.cap_height < m.ascent,
          "x-height and cap height must be ordered below the ascent");
  require(m.underline_thickness > 0.0F && m.underline_position > 0.0F,
          "underline metrics missing");
  const auto line = cgpui::text_line_metrics_for_shape_run(run);
  require(near(line.line_height, run.line_height, 0.01F) &&
              line.baseline >= m.ascent - 0.01F &&
              line.line_height >= m.ascent + m.descent - 0.01F,
          "line metrics are not derived from font metrics");
}

void cjk_fallback_runs() {
  // "AB" + U+4E2D U+6587 (zhong wen) + "CD"
  const std::string text = "AB\xE4\xB8\xAD\xE6\x96\x87" "CD";
  const auto run = cgpui::shape_text(text, proportional, 20.0F);
  require(run.font_runs.size() == 3, "CJK should form its own fallback font run");
  const auto& cjk = run.font_runs[1];
  require(cjk.byte_start == 2 && cjk.byte_end == 8 &&
              cjk.font_fallback_face_index != run.font_runs[0].font_fallback_face_index,
          "CJK fallback run has the wrong byte range or face");
  const auto& face = run.font_fallback_faces[cjk.font_fallback_face_index];
  require(!face.path.empty() && face.font.family != run.font_fallback_faces[0].font.family,
          "CJK fallback face has no native identity");
  for (std::size_t index = cjk.glyph_start; index < cjk.glyph_end; ++index) {
    require(near(run.glyphs[index].advance, 20.0F, 2.0F),
            "CJK ideographs must advance a full em");
    require(run.glyphs[index].glyph_id != 0, "CJK glyph missing in fallback face");
  }
  require(run.missing_glyphs.empty(), "fallback text reported missing glyphs");
  std::cout << "CJK fallback face: " << face.font.family << '\n';
}

void combining_marks_form_one_cluster() {
  // "e" + U+0301 COMBINING ACUTE + "x"
  const auto measurement = cgpui::measure_text("e\xCC\x81x", proportional, 20.0F);
  require(measurement.grapheme_columns.size() == 2, "base + mark must be one grapheme");
  const auto& first = measurement.grapheme_columns[0];
  require(first.byte_start == 0 && first.byte_end == 3 && first.advance > 0.0F,
          "combining grapheme spans the wrong bytes");
  for (std::size_t g = first.glyph_start; g < first.glyph_end; ++g) {
    require(measurement.shape_run.glyphs[g].byte_offset == 0,
            "glyphs of the combined cluster must share its byte range");
  }
  const auto hit = cgpui::hit_test_text_position(
      measurement, {{0, 0}, {200, 40}}, {first.advance * 0.4F, 10});
  require(hit.byte_offset == 0, "hit inside a combined grapheme must not split it");
}

void ligatures_toggle() {
  cgpui::FontDescriptor on = ligature_font;
  cgpui::FontDescriptor off = ligature_font;
  off.features = {cgpui::font_feature("liga", 0)};
  const auto ligated = cgpui::measure_text("office", on, 24.0F);
  const auto plain = cgpui::measure_text("office", off, 24.0F);
  require(plain.shape_run.glyphs.size() == 6, "liga=0 must keep one glyph per letter");
  require(ligated.shape_run.glyphs.size() < 6, "default proportional shaping must ligate ffi");
  require(ligated.grapheme_columns.size() == 6, "ligatures must not merge graphemes");
  // A hit 60 % into the ligature lands inside it on a grapheme boundary.
  float start = column_x(ligated, 1);
  float end = column_x(ligated, 4);
  const auto hit = cgpui::hit_test_text_position(
      ligated, {{0, 0}, {400, 40}}, {start + (end - start) * 0.6F, 10});
  require(hit.byte_offset == 2 || hit.byte_offset == 3,
          "hit testing could not land inside the ffi ligature");
}

void monospace_columns_align() {
  const cgpui::FontDescriptor code = cgpui::default_code_font_descriptor();
  const auto arrow = cgpui::shape_text("a->b!=c", code, 16.0F);
  require(arrow.glyphs.size() == 7, "code font must not ligate -> or != by default");
  for (const auto& glyph : arrow.glyphs) {
    require(near(glyph.advance, arrow.glyphs[0].advance, 0.01F),
            "monospace glyphs must share one advance");
  }
  const float cell = arrow.glyphs[0].advance;
  const auto one = cgpui::measure_text("a\tX", code, 16.0F);
  const auto three = cgpui::measure_text("abc\tX", code, 16.0F);
  const float x1 = column_x(one, 2);
  const float x3 = column_x(three, 4);
  require(near(x1, cell * 4.0F, 0.05F) && near(x3, cell * 4.0F, 0.05F),
          "tab must advance to the next 4-column stop");
  std::cout << "Code font " << code.family << ": cell " << cell << " px\n";
}

void caret_hit_test_round_trip() {
  const std::string text = "Floating \xE4\xB8\xAD\xE6\x96\x87 panel";
  const auto measurement = cgpui::measure_text(text, proportional, 18.0F);
  const cgpui::Rect bounds{{10, 5}, {1000, 40}};
  float x = 0.0F;
  for (const auto& column : measurement.grapheme_columns) {
    const auto caret = cgpui::text_caret_rect(
        bounds, text, proportional, 18.0F, {}, column.byte_start);
    require(near(caret.origin.x, bounds.origin.x + x, 0.01F),
            "caret x is not the sum of shaped grapheme advances");
    const auto hit = cgpui::hit_test_text_position(
        measurement, bounds, {bounds.origin.x + x + 0.25F, 15});
    require(hit.byte_offset == column.byte_start, "hit test did not round-trip the caret");
    x += column.advance;
  }
  require(near(x, measurement.shape_run.total_advance, 0.01F),
          "grapheme advances must sum to the shaped width");
}

void selection_spans_fallback_runs() {
  const std::string text = "AB\xE4\xB8\xAD\xE6\x96\x87" "CD";
  const auto measurement = cgpui::measure_text(text, proportional, 20.0F);
  cgpui::PaintList paints;
  const cgpui::Rect bounds{{0, 0}, {600, 40}};
  cgpui::paint_text_selection_ranges(paints, bounds, cgpui::rgb(0, 0, 255), text,
      proportional, 20.0F, {}, {.start = 1, .end = 9, .collapsed = false});
  require(paints.commands().size() == 1, "one line should give one selection rect");
  const auto rect = paints.commands().front().text_selection.rect;
  require(near(rect.origin.x, column_x(measurement, 1), 0.01F) &&
              near(rect.size.width, column_x(measurement, 9) - column_x(measurement, 1), 0.01F),
          "selection rect does not follow shaped advances across fallback runs");
}

void wrapping_uses_shaped_widths() {
  const auto measurement =
      cgpui::measure_text("Floating panel Floating panel", proportional, 20.0F);
  const float max_width = measurement.shape_run.total_advance * 0.55F;
  const auto layout = cgpui::wrap_text_measurement(measurement, max_width);
  require(layout.lines.size() == 2, "text at 55 % width should wrap onto two lines");
  for (const auto& line : layout.lines) {
    float sum = 0.0F;
    for (std::size_t c = line.column_start; c < line.column_end; ++c) {
      sum += measurement.grapheme_columns[c].advance;
    }
    require(line.size.width <= max_width + 0.01F && near(line.size.width, sum, 0.01F),
            "wrapped line width is not the sum of shaped advances");
  }
  require(near(layout.lines[1].origin.y, measurement.line_metrics.line_height, 0.01F),
          "second line must start one font line height down");
}

void dpi_scales_device_metrics() {
  const auto base = cgpui::shape_text("Floating panel", proportional, 16.0F, {1.0F});
  const auto scaled = cgpui::shape_text("Floating panel", proportional, 16.0F, {1.5F});
  require(near(base.total_advance, scaled.total_advance, 0.01F),
          "logical advances must not depend on DPI");
  require(near(scaled.device_total_advance, scaled.total_advance * 1.5F, 0.01F) &&
              near(scaled.device_line_height, scaled.line_height * 1.5F, 0.01F),
          "device metrics must scale by 1.5");
}

void shaping_is_cached() {
  cgpui::clear_text_shape_cache();
  (void)cgpui::shape_text("cache me", proportional, 16.0F);
  (void)cgpui::shape_text("cache me", proportional, 16.0F);
  (void)cgpui::shape_text("cache me!", proportional, 16.0F);
  const auto stats = cgpui::text_shape_cache_statistics();
  require(stats.hit_count == 1 && stats.miss_count == 2 && stats.entry_count == 2,
          "unchanged text must hit the shape cache and edited text must miss");
  cgpui::set_text_shape_cache_capacity(1);
  require(cgpui::text_shape_cache_statistics().eviction_count == 1,
          "lowering capacity must evict least recently used runs");
  cgpui::set_text_shape_cache_capacity(2048);
  cgpui::TextMeasurementCache measurements;
  (void)measurements.measure("cache me", proportional, 16.0F);
  require(measurements.measure("cache me", proportional, 16.0F).cache_hit,
          "measurement cache missed on unchanged text");
}

void test_fonts_stay_deterministic() {
  cgpui::FontFallbackChain chain;
  const cgpui::FontFaceDescriptor face{
      .font = {.family = "Test"}, .source = cgpui::FontSource::test};
  chain.add_face(&face);
  const auto run = cgpui::shape_text("AB", chain, 18.0F);
  require(run.used_backend == cgpui::TextShapingBackend::deterministic_fallback &&
              run.fallback_reason == cgpui::TextShapingFallbackReason::unsupported_font_source,
          "test font faces must report a typed unsupported font source");
}

} // namespace

int main() {
  try {
    latin_proportional_advances();
    font_metrics_are_real();
    cjk_fallback_runs();
    combining_marks_form_one_cluster();
    ligatures_toggle();
    monospace_columns_align();
    caret_hit_test_round_trip();
    selection_spans_fallback_runs();
    wrapping_uses_shaped_widths();
    dpi_scales_device_metrics();
    shaping_is_cached();
    test_fonts_stay_deterministic();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Native text shaping failed: " << error.what() << '\n';
    return 1;
  }
}

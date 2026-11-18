#include "window_runtime_test_support.hpp"

#include "cgpui/ui/text_composition.hpp"

#include <cmath>
#include <cstdio>

// C8: the text input widget draws the IME composition with one underline
// per clause (thin for input / converted, thick for the selected clause),
// places the caret at the composition caret, and IME candidates follow that
// caret. Uses native shaping (C7 metrics) at scale 1 and 1.5.
namespace {

#if defined(_WIN32)
const char* const family = "Segoe UI";
#else
const char* const family = "DejaVu Sans";
#endif
constexpr float font_size = 20.0F;
const cgpui::Rect bounds{.origin = {.x = 10.0F, .y = 12.0F},
                         .size = {.width = 400.0F, .height = 40.0F}};

bool near(float lhs, float rhs) {
  return std::fabs(lhs - rhs) < 0.01F;
}

cgpui::Style input_style() {
  cgpui::Style style = cgpui::Style{}.with_font_size(font_size);
  style.font.family = family;
  return style;
}

// x of a byte offset from the shaped grapheme advances of `text`.
float x_at(std::string_view text, std::size_t offset, float scale,
           float origin = bounds.origin.x) {
  const cgpui::TextMeasurement measurement = cgpui::measure_text(
      text, cgpui::FontDescriptor{.family = family}, font_size, {scale});
  float x = origin;
  for (const auto& column : measurement.grapheme_columns) {
    if (column.byte_start >= offset) break;
    x += column.advance;
  }
  return x;
}

// "xy" converted, U+4E2D the selected (target) clause, caret after "xy".
cgpui::ImeComposition clause_composition() {
  cgpui::ImeComposition composition{
      .phase = cgpui::ImeCompositionPhase::update,
      .text = "xy\xE4\xB8\xAD",
      .preedit_cursor_begin = 2,
      .preedit_cursor_end = 2,
  };
  (void)cgpui::append_ime_preedit_style(composition, {
      .byte_offset = 0, .byte_length = 2,
      .clause = cgpui::ImeClauseStyle::converted});
  (void)cgpui::append_ime_preedit_style(composition, {
      .byte_offset = 2, .byte_length = 3,
      .clause = cgpui::ImeClauseStyle::selected_converted});
  return composition;
}

int test_normalization() {
  const cgpui::TextComposition clauses =
      cgpui::text_composition_from_ime(clause_composition());
  if (clauses.caret_begin != 2 || clauses.caret_end != 2 ||
      !clauses.caret_visible || clauses.clauses.size() != 2 ||
      clauses.clauses[1].byte_offset != 2 ||
      clauses.clauses[1].byte_length != 3 ||
      clauses.clauses[1].style != cgpui::ImeClauseStyle::selected_converted) {
    return 1;
  }
  // Offsets inside U+4E2D snap back to its first byte; a hidden caret.
  cgpui::ImeComposition odd{.phase = cgpui::ImeCompositionPhase::update,
                            .text = "a\xE4\xB8\xAD",
                            .preedit_cursor_begin = 3,
                            .preedit_cursor_end = 9};
  (void)cgpui::append_ime_preedit_style(odd, {.byte_offset = 2,
                                              .byte_length = 1});
  const cgpui::TextComposition snapped = cgpui::text_composition_from_ime(odd);
  if (snapped.caret_begin != 1 || snapped.caret_end != 4 ||
      snapped.clauses.size() != 1 || snapped.clauses[0].byte_offset != 0 ||
      snapped.clauses[0].byte_length != 4) {
    return 2;  // empty span dropped, whole-text input clause instead
  }
  odd.preedit_cursor_begin = -1;
  odd.preedit_cursor_end = -1;
  if (cgpui::text_composition_from_ime(odd).caret_visible) return 3;
  const cgpui::TextComposition plain =
      cgpui::text_composition_from_text("pin");
  if (plain.caret_end != 3 || plain.clauses.size() != 1 ||
      plain.clauses[0].style != cgpui::ImeClauseStyle::input) {
    return 4;
  }
  cgpui::TextModel model("ab");
  model.set_composition(plain);
  model.commit_composition();
  return model.text() == "abpin" && !model.has_composition() ? 0 : 5;
}

int test_widget_paint(float scale) {
  cgpui::TextModel model("ab");
  cgpui::TextInputElement input(model, input_style());
  input.set_layout_bounds(bounds);
  model.set_composition(cgpui::text_composition_from_ime(clause_composition()));

  cgpui::PaintList paints;
  paints.set_scale({scale});
  input.paint(paints);
  const std::string display = "abxy\xE4\xB8\xAD";
  std::vector<cgpui::Rect> underlines;
  std::optional<cgpui::Rect> caret;
  bool text_ok = false;
  for (const cgpui::PaintCommand& command : paints.commands()) {
    if (command.kind == cgpui::PaintCommandKind::solid_rect) {
      underlines.push_back(command.solid_rect.rect);
    } else if (command.kind == cgpui::PaintCommandKind::text_caret) {
      caret = command.text_caret.rect;
    } else if (command.kind == cgpui::PaintCommandKind::text) {
      text_ok = command.text.content == display;
    }
  }
  if (!text_ok || underlines.size() != 2 || !caret.has_value()) return 10;
  const float pixel = 1.0F / scale;
  const cgpui::Rect& thin = underlines[0];
  const cgpui::Rect& thick = underlines[1];
  std::printf("scale %.1f: converted %.3f..%.3f h %.3f, selected %.3f..%.3f "
              "h %.3f, caret %.3f\n", scale, thin.origin.x,
              thin.origin.x + thin.size.width, thin.size.height,
              thick.origin.x, thick.origin.x + thick.size.width,
              thick.size.height, caret->origin.x);
  if (!near(thin.origin.x, x_at(display, 2, scale)) ||
      !near(thin.origin.x + thin.size.width, x_at(display, 4, scale) - pixel) ||
      !near(thick.origin.x, x_at(display, 4, scale)) ||
      !near(thick.origin.x + thick.size.width,
            x_at(display, 7, scale) - pixel)) {
    return 11;
  }
  if (!near(thick.size.height, 2.0F * thin.size.height) ||
      thin.size.height < pixel - 0.001F) {
    return 12;
  }
  // Underlines sit below the baseline, inside the line box.
  const float line_bottom = bounds.origin.y + caret->size.height;
  if (thin.origin.y <= bounds.origin.y + font_size * 0.5F ||
      thick.origin.y + thick.size.height > line_bottom + 0.001F) {
    return 13;
  }
  // Composition caret after "xy", not at the model cursor after "ab".
  if (!near(caret->origin.x, x_at(display, 4, scale))) return 14;
  const auto start = input.composition_start_rect({scale});
  const auto ime_caret = input.caret_rect({scale});
  if (!start.has_value() || !near(start->origin.x, x_at(display, 2, scale)) ||
      !ime_caret.has_value() || !near(ime_caret->origin.x, caret->origin.x)) {
    return 15;
  }
  return 0;
}

int test_hidden_caret_paints_no_caret() {
  cgpui::TextModel model("ab");
  cgpui::TextInputElement input(model, input_style());
  input.set_layout_bounds(bounds);
  cgpui::ImeComposition composition = clause_composition();
  composition.preedit_cursor_begin = -1;
  composition.preedit_cursor_end = -1;
  model.set_composition(cgpui::text_composition_from_ime(composition));
  cgpui::PaintList paints;
  input.paint(paints);
  for (const cgpui::PaintCommand& command : paints.commands()) {
    if (command.kind == cgpui::PaintCommandKind::text_caret) return 20;
  }
  return 0;
}

RuntimeFixture* placement_fixture = nullptr;

void dispatch_composition_placement_sequence() {
  auto& callback = placement_fixture->window.callback;
  callback(cgpui::KeyboardKey{.key_code = 84,
                              .action = cgpui::KeyAction::pressed});
  callback(clause_composition());
  // A frame lays the element out again with the composition text.
  callback(cgpui::WindowRedrawRequested{});
}

// Candidates (CFS_EXCLUDE rect, text-input v3 cursor rectangle) follow the
// composition caret; the composition window anchors at the start.
int test_candidate_placement_follows_composition_caret() {
  RuntimeFixture fixture;
  placement_fixture = &fixture;
  fixture.app.on_run = &dispatch_composition_placement_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{1};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("ab");
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId text_id = tree->set_root(
      std::make_unique<cgpui::TextElement>(&model, input_style()));
  auto* element = tree->find_as<cgpui::TextElement>(text_id);
  if (element == nullptr) return 30;
  element->set_layout_bounds(bounds);

  cgpui::WindowRuntime runtime(
      fixture.app, fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(text_id, &model);
  runtime.set_element_tree(std::move(tree));
  const int result = runtime.run(cgpui::WindowDescriptor{});
  placement_fixture = nullptr;
  if (result != 0 || !model.has_composition()) return 31;

  std::optional<cgpui::ImeTextInputPlacement> last;
  for (const auto& placement : fixture.window.ime_placement_history) {
    if (placement.has_value()) last = placement;
  }
  const std::string display = "abxy\xE4\xB8\xAD";
  if (!last.has_value() || !last->candidate_rect.has_value()) return 32;
  std::printf("placement rect x %.3f, candidate x %.3f\n", last->rect.origin.x,
              last->candidate_rect->origin.x);
  // The runtime lays the element out again; offsets are from its bounds,
  // which now fit the whole composition on one line.
  const auto laid_out = element->layout_bounds();
  if (!laid_out.has_value()) return 34;
  const float origin = laid_out->origin.x;
  if (laid_out->size.width < x_at(display, 7, 1.0F, 0.0F) - 0.01F) return 35;
  if (!near(last->rect.origin.x, x_at(display, 2, 1.0F, origin)) ||
      !near(last->candidate_rect->origin.x, x_at(display, 4, 1.0F, origin)) ||
      !near(last->rect.origin.y, last->candidate_rect->origin.y) ||
      last->byte_offset != 2) {
    return 33;
  }
  return 0;
}

} // namespace

int main() {
  for (const int result :
       {test_normalization(), test_widget_paint(1.0F), test_widget_paint(1.5F),
        test_hidden_caret_paints_no_caret(),
        test_candidate_placement_follows_composition_caret()}) {
    if (result != 0) {
      std::fprintf(stderr, "text composition paint test failed: %d\n", result);
      return result;
    }
  }
  return 0;
}

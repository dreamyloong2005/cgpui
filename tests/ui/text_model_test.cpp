#include "cgpui/ui/text.hpp"

#include <span>
#include <string_view>
#include <vector>

namespace {

int test_text_model_defaults_to_empty_buffer() {
  const cgpui::TextModel model;
  if (!model.text().empty()) {
    return 1;
  }
  return model.cursor() == 0 ? 0 : 2;
}

int test_text_model_inserts_utf8_at_cursor() {
  cgpui::TextModel model;
  model.insert_text("a");
  model.insert_text("\xE4\xB8\xAD");
  model.insert_text("b");

  if (model.text() != std::string_view{"a" "\xE4\xB8\xAD" "b"}) {
    return 3;
  }
  if (model.cursor() != model.text().size()) {
    return 4;
  }

  (void)model.move_cursor_previous();
  model.insert_text("-");
  return model.text() == std::string_view{"a" "\xE4\xB8\xAD" "-b"} &&
                 model.cursor() ==
                     std::string_view{"a" "\xE4\xB8\xAD" "-"}.size()
             ? 0
             : 5;
}

int test_text_model_moves_cursor_by_utf8_codepoints() {
  cgpui::TextModel model("a" "\xE4\xB8\xAD" "b");

  if (model.cursor() != std::string_view{"a" "\xE4\xB8\xAD" "b"}.size()) {
    return 6;
  }
  (void)model.move_cursor_previous();
  if (model.cursor() != std::string_view{"a" "\xE4\xB8\xAD"}.size()) {
    return 7;
  }
  (void)model.move_cursor_previous();
  if (model.cursor() != std::string_view{"a"}.size()) {
    return 8;
  }
  (void)model.move_cursor_next();
  return model.cursor() == std::string_view{"a" "\xE4\xB8\xAD"}.size() ? 0
                                                                       : 9;
}

int test_text_model_moves_cursor_by_grapheme_clusters() {
  cgpui::TextModel combining_mark("a\xCC\x81" "b");
  if (combining_mark.cursor() != std::string_view{"a\xCC\x81" "b"}.size()) {
    return 90;
  }
  if (!combining_mark.move_cursor_previous() ||
      combining_mark.cursor() != std::string_view{"a\xCC\x81"}.size()) {
    return 91;
  }
  if (!combining_mark.move_cursor_previous() ||
      combining_mark.cursor() != 0) {
    return 92;
  }
  if (!combining_mark.move_cursor_next() ||
      combining_mark.cursor() != std::string_view{"a\xCC\x81"}.size()) {
    return 93;
  }

  cgpui::TextModel flag("\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8" "!");
  if (!flag.move_cursor_previous() ||
      flag.cursor() !=
          std::string_view{"\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8"}.size()) {
    return 94;
  }
  if (!flag.move_cursor_previous() || flag.cursor() != 0) {
    return 95;
  }

  cgpui::TextModel zwj("\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB" "x");
  if (!zwj.move_cursor_previous() ||
      zwj.cursor() !=
          std::string_view{
              "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB"}.size()) {
    return 96;
  }
  if (!zwj.move_cursor_previous() || zwj.cursor() != 0) {
    return 97;
  }
  if (!zwj.move_cursor_next() ||
      zwj.cursor() !=
          std::string_view{
              "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB"}.size()) {
    return 98;
  }

  return 0;
}

int test_text_model_backspace_deletes_previous_utf8_codepoint() {
  cgpui::TextModel model("a" "\xE4\xB8\xAD" "b");
  (void)model.move_cursor_previous();
  if (!model.backspace()) {
    return 10;
  }
  if (model.text() != std::string_view{"ab"} ||
      model.cursor() != std::string_view{"a"}.size()) {
    return 11;
  }
  if (!model.backspace()) {
    return 12;
  }
  return model.text() == std::string_view{"b"} && model.cursor() == 0 ? 0 : 13;
}

int test_text_model_delete_removes_next_utf8_codepoint() {
  cgpui::TextModel model("a" "\xE4\xB8\xAD" "b");
  (void)model.move_cursor_previous();
  (void)model.move_cursor_previous();

  if (!model.delete_forward()) {
    return 14;
  }
  if (model.text() != std::string_view{"ab"} ||
      model.cursor() != std::string_view{"a"}.size()) {
    return 15;
  }
  (void)model.move_cursor_next();
  return model.delete_forward() ? 16 : 0;
}

int test_text_model_tracks_selection_range() {
  cgpui::TextModel model("abcd");
  if (!model.selection().collapsed || model.selection().start != model.cursor() ||
      model.selection().end != model.cursor()) {
    return 17;
  }

  model.set_selection(3, 1);
  if (model.cursor() != 1 || model.selection_anchor() != 3 ||
      model.selection_head() != 1 || model.selection().start != 1 ||
      model.selection().end != 3 || model.selection().collapsed ||
      model.selected_text() != std::string_view{"bc"}) {
    return 18;
  }

  model.set_selection(99, 2);
  if (model.selection_anchor() != model.text().size() ||
      model.selection_head() != 2 || model.selection().start != 2 ||
      model.selection().end != model.text().size()) {
    return 19;
  }

  model.clear_selection();
  if (!model.selection().collapsed || model.selection().start != model.cursor() ||
      model.selection().end != model.cursor() || !model.selected_text().empty()) {
    return 20;
  }

  model.set_selection(1, 3);
  (void)model.move_cursor_previous();
  if (!model.selection().collapsed || model.selection().start != model.cursor()) {
    return 21;
  }

  model.set_selection(1, 3);
  model.insert_text("Z");
  if (model.text() != std::string_view{"aZd"} || model.cursor() != 2 ||
      !model.selection().collapsed) {
    return 22;
  }

  return 0;
}

int test_text_model_applies_key_edit_actions() {
  cgpui::TextModel model("a" "\xE4\xB8\xAD" "b");

  if (!model.apply_edit_action(cgpui::TextEditAction::move_previous) ||
      model.cursor() != std::string_view{"a" "\xE4\xB8\xAD"}.size()) {
    return 23;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::extend_previous) ||
      model.selection_anchor() != std::string_view{"a" "\xE4\xB8\xAD"}.size() ||
      model.selection_head() != std::string_view{"a"}.size() ||
      model.selection().start != std::string_view{"a"}.size() ||
      model.selection().end != std::string_view{"a" "\xE4\xB8\xAD"}.size()) {
    return 24;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::extend_next) ||
      model.selection_head() != std::string_view{"a" "\xE4\xB8\xAD"}.size() ||
      !model.selection().collapsed) {
    return 25;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::extend_next) ||
      model.selection_anchor() != std::string_view{"a" "\xE4\xB8\xAD"}.size() ||
      model.selection_head() != model.text().size() ||
      model.selection().start != std::string_view{"a" "\xE4\xB8\xAD"}.size() ||
      model.selection().end != model.text().size()) {
    return 26;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::delete_forward) ||
      model.text() != std::string_view{"a" "\xE4\xB8\xAD"} ||
      model.cursor() != std::string_view{"a" "\xE4\xB8\xAD"}.size() ||
      !model.selection().collapsed) {
    return 27;
  }

  model.set_selection(1, model.text().size());
  if (!model.apply_edit_action(cgpui::TextEditAction::backspace) ||
      model.text() != std::string_view{"a"} || model.cursor() != 1 ||
      !model.selection().collapsed) {
    return 28;
  }
  if (model.apply_edit_action(cgpui::TextEditAction::move_next)) {
    return 29;
  }

  return 0;
}

int test_text_model_applies_word_edit_actions() {
  cgpui::TextModel model("alpha  beta\tgamma");
  const std::size_t alpha_start = 0;
  const std::size_t beta_start = std::string_view{"alpha  "}.size();
  const std::size_t beta_end = std::string_view{"alpha  beta"}.size();
  const std::size_t gamma_start = std::string_view{"alpha  beta\t"}.size();

  if (!model.apply_edit_action(cgpui::TextEditAction::move_previous_word) ||
      model.cursor() != gamma_start || !model.selection().collapsed) {
    return 99;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::move_previous_word) ||
      model.cursor() != beta_start || !model.selection().collapsed) {
    return 100;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::move_next_word) ||
      model.cursor() != beta_end || !model.selection().collapsed) {
    return 101;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::move_next_word) ||
      model.cursor() != model.text().size() || !model.selection().collapsed) {
    return 102;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::move_previous_word) ||
      model.cursor() != gamma_start) {
    return 103;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::move_previous_word) ||
      model.cursor() != beta_start) {
    return 104;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::move_previous_word) ||
      model.cursor() != alpha_start) {
    return 105;
  }

  cgpui::TextModel forward_selection("one two");
  forward_selection.set_selection(0, 0);
  if (!forward_selection.apply_edit_action(
          cgpui::TextEditAction::extend_next_word) ||
      forward_selection.selection_anchor() != 0 ||
      forward_selection.selection_head() != std::string_view{"one"}.size() ||
      forward_selection.selected_text() != std::string_view{"one"}) {
    return 106;
  }
  if (!forward_selection.apply_edit_action(
          cgpui::TextEditAction::extend_next_word) ||
      forward_selection.selection_anchor() != 0 ||
      forward_selection.selection_head() != forward_selection.text().size() ||
      forward_selection.selected_text() != std::string_view{"one two"}) {
    return 107;
  }

  cgpui::TextModel backward_selection("one two");
  if (!backward_selection.apply_edit_action(
          cgpui::TextEditAction::extend_previous_word) ||
      backward_selection.selection_anchor() != backward_selection.text().size() ||
      backward_selection.selection_head() != std::string_view{"one "}.size() ||
      backward_selection.selected_text() != std::string_view{"two"}) {
    return 108;
  }
  if (!backward_selection.apply_edit_action(
          cgpui::TextEditAction::extend_previous_word) ||
      backward_selection.selection_anchor() != backward_selection.text().size() ||
      backward_selection.selection_head() != 0 ||
      backward_selection.selected_text() != std::string_view{"one two"}) {
    return 109;
  }

  cgpui::TextModel unicode_space("wide\xE3\x80\x80gap");
  if (!unicode_space.apply_edit_action(
          cgpui::TextEditAction::move_previous_word) ||
      unicode_space.cursor() != std::string_view{"wide\xE3\x80\x80"}.size()) {
    return 110;
  }
  if (!unicode_space.apply_edit_action(
          cgpui::TextEditAction::move_previous_word) ||
      unicode_space.cursor() != 0) {
    return 111;
  }

  return 0;
}

int test_text_model_navigates_multiline_lines() {
  cgpui::TextModel model("abc\ndefg\nhi");

  if (model.line_count() != 3 || model.line_index_at(0) != 0 ||
      model.line_index_at(4) != 1 || model.line_index_at(model.text().size()) != 2 ||
      model.line_start_offset(6) != 4 || model.line_end_offset(6) != 8 ||
      model.line_start_offset(model.text().size()) != 9 ||
      model.line_end_offset(model.text().size()) != model.text().size()) {
    return 132;
  }

  model.set_selection(6, 6);
  if (!model.apply_edit_action(cgpui::TextEditAction::move_line_start) ||
      model.cursor() != 4 || !model.selection().collapsed) {
    return 133;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::move_line_end) ||
      model.cursor() != 8 || !model.selection().collapsed) {
    return 134;
  }

  model.set_selection(7, 7);
  if (!model.apply_edit_action(cgpui::TextEditAction::move_previous_line) ||
      model.cursor() != 3 || !model.selection().collapsed) {
    return 135;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::move_next_line) ||
      model.cursor() != 7 || !model.selection().collapsed) {
    return 136;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::move_next_line) ||
      model.cursor() != model.text().size() || !model.selection().collapsed) {
    return 137;
  }
  if (model.apply_edit_action(cgpui::TextEditAction::move_next_line)) {
    return 138;
  }

  cgpui::TextModel selection("abc\ndefg\nhi");
  selection.set_selection(4, 4);
  if (!selection.apply_edit_action(cgpui::TextEditAction::extend_line_end) ||
      selection.selection_anchor() != 4 || selection.selection_head() != 8 ||
      selection.selected_text() != std::string_view{"defg"}) {
    return 139;
  }
  if (!selection.apply_edit_action(cgpui::TextEditAction::extend_next_line) ||
      selection.selection_anchor() != 4 ||
      selection.selection_head() != selection.text().size() ||
      selection.selected_text() != std::string_view{"defg\nhi"}) {
    return 140;
  }

  cgpui::TextModel backward_selection("abc\ndefg\nhi");
  backward_selection.set_selection(10, 10);
  if (!backward_selection.apply_edit_action(
          cgpui::TextEditAction::extend_previous_line) ||
      backward_selection.selection_anchor() != 10 ||
      backward_selection.selection_head() != 5 ||
      backward_selection.selected_text() != std::string_view{"efg\nh"}) {
    return 141;
  }
  if (!backward_selection.apply_edit_action(
          cgpui::TextEditAction::extend_line_start) ||
      backward_selection.selection_anchor() != 10 ||
      backward_selection.selection_head() != 4 ||
      backward_selection.selected_text() != std::string_view{"defg\nh"}) {
    return 142;
  }

  return 0;
}

int test_text_model_undo_redo_restores_edit_history() {
  cgpui::TextModel model;
  if (model.can_undo() || model.can_redo() || model.undo() || model.redo()) {
    return 112;
  }

  model.insert_text("hello");
  if (!model.can_undo() || model.can_redo() ||
      model.text() != std::string_view{"hello"} || model.cursor() != 5) {
    return 113;
  }
  if (!model.undo() || model.can_undo() || !model.can_redo() ||
      !model.text().empty() || model.cursor() != 0 ||
      !model.selection().collapsed) {
    return 114;
  }
  if (!model.redo() || !model.can_undo() || model.can_redo() ||
      model.text() != std::string_view{"hello"} || model.cursor() != 5 ||
      !model.selection().collapsed) {
    return 115;
  }

  model.set_selection(1, 4);
  model.insert_text("i");
  if (model.text() != std::string_view{"hio"} || model.cursor() != 2 ||
      !model.selection().collapsed) {
    return 116;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::undo) ||
      model.text() != std::string_view{"hello"} || model.cursor() != 4 ||
      model.selection_anchor() != 1 || model.selection_head() != 4 ||
      model.selected_text() != std::string_view{"ell"}) {
    return 117;
  }
  if (!model.apply_edit_action(cgpui::TextEditAction::redo) ||
      model.text() != std::string_view{"hio"} || model.cursor() != 2 ||
      !model.selection().collapsed) {
    return 118;
  }

  model.set_selection(1, 2);
  if (!model.delete_forward() || model.text() != std::string_view{"ho"} ||
      model.cursor() != 1 || !model.selection().collapsed) {
    return 119;
  }
  if (!model.undo() || model.text() != std::string_view{"hio"} ||
      model.selection_anchor() != 1 || model.selection_head() != 2 ||
      model.selected_text() != std::string_view{"i"}) {
    return 120;
  }
  if (!model.redo() || model.text() != std::string_view{"ho"} ||
      model.cursor() != 1 || !model.selection().collapsed) {
    return 121;
  }

  cgpui::TextModel composition("ab");
  composition.set_composition_text("\xE4\xB8\xAD");
  composition.commit_composition();
  if (composition.has_composition() ||
      composition.text() != std::string_view{"ab\xE4\xB8\xAD"} ||
      composition.cursor() != composition.text().size()) {
    return 122;
  }
  if (!composition.undo() || composition.has_composition() ||
      composition.text() != std::string_view{"ab"} ||
      composition.cursor() != std::string_view{"ab"}.size()) {
    return 123;
  }
  if (!composition.redo() || composition.has_composition() ||
      composition.text() != std::string_view{"ab\xE4\xB8\xAD"} ||
      composition.cursor() != composition.text().size()) {
    return 124;
  }

  if (!composition.undo()) {
    return 125;
  }
  composition.insert_text("!");
  if (composition.can_redo() ||
      composition.text() != std::string_view{"ab!"}) {
    return 126;
  }

  return 0;
}

int test_text_model_deletes_surrounding_text_on_utf8_boundaries() {
  const std::string text_with_zhong =
      std::string{"ab"} + "\xE4\xB8\xAD" + "cd";

  cgpui::TextModel model(text_with_zhong);
  model.set_selection(5, 5);
  if (!model.delete_surrounding_text(1, 1) ||
      model.text() != std::string_view{"abd"} || model.cursor() != 2 ||
      !model.selection().collapsed) {
    return 127;
  }
  if (!model.undo() || model.text() != std::string_view{text_with_zhong} ||
      model.cursor() != 5) {
    return 128;
  }

  cgpui::TextModel forward(text_with_zhong);
  forward.set_selection(2, 2);
  if (!forward.delete_surrounding_text(0, 1) ||
      forward.text() != std::string_view{"abcd"} ||
      forward.cursor() != 2) {
    return 129;
  }

  cgpui::TextModel clamped("abc");
  clamped.set_selection(1, 1);
  if (!clamped.delete_surrounding_text(100, 100) ||
      !clamped.text().empty() || clamped.cursor() != 0) {
    return 130;
  }
  if (clamped.delete_surrounding_text(0, 0)) {
    return 131;
  }

  return 0;
}

int test_text_model_tracks_ime_composition() {
  cgpui::TextModel model("ab");

  model.set_composition_text("preedit");
  if (!model.has_composition() ||
      model.composition_text() != std::string_view{"preedit"} ||
      model.text() != std::string_view{"ab"}) {
    return 30;
  }

  model.set_composition_text("\xE4\xB8\xAD");
  if (model.composition_text() != std::string_view{"\xE4\xB8\xAD"} ||
      model.text() != std::string_view{"ab"}) {
    return 31;
  }

  model.commit_composition();
  if (model.has_composition() || model.text() != std::string_view{"ab\xE4\xB8\xAD"} ||
      model.cursor() != model.text().size()) {
    return 32;
  }

  model.set_composition_text("draft");
  model.cancel_composition();
  return !model.has_composition() && model.text() == std::string_view{"ab\xE4\xB8\xAD"}
             ? 0
             : 33;
}

int test_font_database_registers_and_resolves_faces() {
  cgpui::FontDatabase database;
  if (!database.empty() || database.face_count() != 0 ||
      !database.faces().empty()) {
    return 34;
  }

  const cgpui::FontFaceDescriptor face{
      .font = cgpui::FontDescriptor{.family = "Inter"},
      .postscript_name = "Inter-Regular",
      .source = cgpui::FontSource::test,
      .path = "fixtures/Inter-Regular.ttf",
  };
  database.add_face(face);
  database.add_face(face);

  if (database.empty() || database.face_count() != 1) {
    return 35;
  }
  const cgpui::FontFaceDescriptor* resolved =
      database.resolve(cgpui::FontDescriptor{.family = "Inter"});
  if (resolved == nullptr || resolved->postscript_name != "Inter-Regular" ||
      resolved->source != cgpui::FontSource::test ||
      resolved->path != "fixtures/Inter-Regular.ttf") {
    return 36;
  }
  if (database.resolve(cgpui::FontDescriptor{.family = "Missing"}) != nullptr) {
    return 37;
  }

  return 0;
}

int test_font_database_resolves_ordered_fallback_chain() {
  cgpui::FontDatabase database;
  database.add_face(cgpui::FontFaceDescriptor{
      .font = cgpui::FontDescriptor{.family = "System UI"},
      .postscript_name = "SystemUI-Regular",
      .source = cgpui::FontSource::test,
      .path = "system-ui.ttf",
  });
  database.add_face(cgpui::FontFaceDescriptor{
      .font = cgpui::FontDescriptor{.family = "Noto Sans"},
      .postscript_name = "NotoSans-Regular",
      .source = cgpui::FontSource::test,
      .path = "noto-sans.ttf",
  });
  database.add_face(cgpui::FontFaceDescriptor{
      .font = cgpui::FontDescriptor{.family = "Inter"},
      .postscript_name = "Inter-Regular",
      .source = cgpui::FontSource::test,
      .path = "inter.ttf",
  });
  database.add_generic_fallback_family("Noto Sans");
  database.add_generic_fallback_family("System UI");

  const cgpui::FontFallbackChain inter_chain =
      database.resolve_chain(cgpui::FontDescriptor{.family = "Inter"});
  if (inter_chain.empty() || inter_chain.size() != 3 ||
      inter_chain.primary()->postscript_name != "Inter-Regular" ||
      inter_chain.faces()[1]->postscript_name != "NotoSans-Regular" ||
      inter_chain.faces()[2]->postscript_name != "SystemUI-Regular") {
    return 38;
  }

  const cgpui::FontFallbackChain missing_chain =
      database.resolve_chain(cgpui::FontDescriptor{.family = "Missing"});
  if (missing_chain.empty() || missing_chain.size() != 2 ||
      missing_chain.primary()->postscript_name != "NotoSans-Regular" ||
      missing_chain.faces()[1]->postscript_name != "SystemUI-Regular") {
    return 39;
  }

  const cgpui::FontFallbackChain empty_chain =
      database.resolve_chain(cgpui::FontDescriptor{});
  if (empty_chain.empty() || empty_chain.size() != 1 ||
      empty_chain.primary()->postscript_name != "SystemUI-Regular") {
    return 40;
  }

  return 0;
}

int test_fake_font_discovery_is_deterministic() {
  const std::vector<cgpui::FontFaceDescriptor> fixtures{
      cgpui::FontFaceDescriptor{
          .font = cgpui::FontDescriptor{.family = "Zed Sans"},
          .postscript_name = "ZedSans-Regular",
          .source = cgpui::FontSource::test,
          .path = "zed-sans.ttf",
      },
      cgpui::FontFaceDescriptor{
          .font = cgpui::FontDescriptor{.family = "Zed Mono"},
          .postscript_name = "ZedMono-Regular",
          .source = cgpui::FontSource::test,
          .path = "zed-mono.ttf",
      },
  };

  const cgpui::FontDatabase database =
      cgpui::discover_test_fonts(std::span<const cgpui::FontFaceDescriptor>(
          fixtures.data(),
          fixtures.size()));
  if (database.face_count() != fixtures.size()) {
    return 41;
  }
  if (database.faces()[0].font.family != "Zed Sans" ||
      database.faces()[1].font.family != "Zed Mono") {
    return 42;
  }
  return database.resolve(cgpui::FontDescriptor{.family = "Zed Mono"}) !=
                 nullptr
             ? 0
             : 43;
}

int test_platform_font_discovery_records_preserve_metadata() {
  const std::vector<cgpui::FontFaceDescriptor> records{
      cgpui::FontFaceDescriptor{
          .font = cgpui::FontDescriptor{.family = "Platform Sans"},
          .postscript_name = "PlatformSans-Regular",
          .source = cgpui::FontSource::platform,
          .path = "platform://sans",
      },
  };

  const cgpui::FontDatabase database =
      cgpui::font_database_from_discovered_faces(
          std::span<const cgpui::FontFaceDescriptor>(
              records.data(),
              records.size()));
  const cgpui::FontFaceDescriptor* resolved =
      database.resolve(cgpui::FontDescriptor{.family = "Platform Sans"});
  if (database.face_count() != 1 || resolved == nullptr) {
    return 90;
  }
  if (resolved->source != cgpui::FontSource::platform ||
      resolved->postscript_name != "PlatformSans-Regular" ||
      resolved->path != "platform://sans") {
    return 91;
  }

  return 0;
}

int test_shape_text_produces_deterministic_fallback_glyphs() {
  const cgpui::TextShapeRun run = cgpui::shape_text(
      "A\xE4\xB8\xAD",
      cgpui::FontDescriptor{.family = "Inter"},
      20.0F);

  if (run.text != std::string_view{"A\xE4\xB8\xAD"} ||
      run.font.family != "Inter" || run.font_size != 20.0F ||
      run.glyph_count() != 2 || run.byte_length != 4) {
    return 44;
  }
  if (run.glyphs[0].byte_offset != 0 || run.glyphs[0].byte_length != 1 ||
      run.glyphs[0].advance != 10.0F) {
    return 45;
  }
  if (run.glyphs[1].byte_offset != 1 || run.glyphs[1].byte_length != 3 ||
      run.glyphs[1].advance != 10.0F) {
    return 46;
  }
  return run.total_advance == 20.0F && run.line_height == 20.0F ? 0 : 47;
}

int test_text_measurement_cache_reuses_same_text_tuple() {
  cgpui::TextMeasurementCache cache;
  const cgpui::FontDescriptor font{.family = "Inter"};
  const cgpui::DpiScale scale{.value = 2.0F};

  const cgpui::TextMeasurementResult first =
      cache.measure("Cache", font, 18.0F, scale);
  if (first.cache_hit || cache.entry_count() != 1 ||
      cache.miss_count() != 1 || cache.hit_count() != 0) {
    return 54;
  }
  if (first.measurement.shape_run.text != "Cache" ||
      first.measurement.shape_run.font.family != "Inter" ||
      first.measurement.logical_size.width != 45.0F ||
      first.measurement.logical_size.height != 18.0F ||
      first.measurement.device_size.width != 90.0F ||
      first.measurement.device_size.height != 36.0F) {
    return 55;
  }

  const cgpui::TextMeasurementResult second =
      cache.measure("Cache", font, 18.0F, scale);
  if (!second.cache_hit || cache.entry_count() != 1 ||
      cache.miss_count() != 1 || cache.hit_count() != 1) {
    return 56;
  }
  if (second.measurement.shape_run.glyph_count() !=
          first.measurement.shape_run.glyph_count() ||
      second.measurement.device_size.width !=
          first.measurement.device_size.width) {
    return 57;
  }

  const cgpui::TextMeasurementResult different_scale =
      cache.measure("Cache", font, 18.0F, cgpui::DpiScale{.value = 1.5F});
  if (different_scale.cache_hit || cache.entry_count() != 2 ||
      cache.miss_count() != 2 || cache.hit_count() != 1 ||
      different_scale.measurement.device_size.width != 67.5F) {
    return 58;
  }

  const cgpui::TextMeasurementResult different_text =
      cache.measure("Cached", font, 18.0F, scale);
  return !different_text.cache_hit && cache.entry_count() == 3 &&
                 cache.miss_count() == 3 && cache.hit_count() == 1
             ? 0
             : 59;
}

int test_text_hit_geometry_maps_points_to_offsets_and_selection() {
  const cgpui::TextMeasurement measurement = cgpui::measure_text(
      "abcd",
      cgpui::FontDescriptor{.family = "Inter"},
      20.0F);
  const cgpui::Rect bounds{
      .origin = {.x = 10.0F, .y = 5.0F},
      .size = {.width = 40.0F, .height = 20.0F},
  };

  const cgpui::TextHitTestResult before =
      cgpui::hit_test_text_position(measurement, bounds, {4.0F, 10.0F});
  if (before.byte_offset != 0 || before.inside_bounds) {
    return 60;
  }

  const cgpui::TextHitTestResult near_first =
      cgpui::hit_test_text_position(measurement, bounds, {14.0F, 10.0F});
  const cgpui::TextHitTestResult after_first =
      cgpui::hit_test_text_position(measurement, bounds, {16.0F, 10.0F});
  const cgpui::TextHitTestResult after_third =
      cgpui::hit_test_text_position(measurement, bounds, {39.0F, 10.0F});
  const cgpui::TextHitTestResult after_text =
      cgpui::hit_test_text_position(measurement, bounds, {60.0F, 10.0F});
  if (near_first.byte_offset != 0 || !near_first.inside_bounds ||
      after_first.byte_offset != 1 || !after_first.inside_bounds ||
      after_third.byte_offset != 3 || !after_third.inside_bounds ||
      after_text.byte_offset != 4 || after_text.inside_bounds) {
    return 61;
  }

  const cgpui::TextSelectionRange forward =
      cgpui::text_selection_range_from_points(
          measurement,
          bounds,
          {11.0F, 10.0F},
          {39.0F, 10.0F});
  if (forward.start != 0 || forward.end != 3 || forward.collapsed) {
    return 62;
  }

  const cgpui::TextSelectionRange backward =
      cgpui::text_selection_range_from_points(
          measurement,
          bounds,
          {49.0F, 10.0F},
          {24.0F, 10.0F});
  return backward.start == 1 && backward.end == 4 && !backward.collapsed ? 0
                                                                         : 63;
}

int test_fallback_glyph_rasterizer_produces_deterministic_bitmap() {
  const cgpui::TextShapeRun run = cgpui::shape_text(
      "A\xE4\xB8\xAD",
      cgpui::FontDescriptor{.family = "Inter"},
      20.0F);
  const std::vector<cgpui::TextGlyphPaint> glyphs =
      cgpui::text_glyph_paint_metadata(run);
  if (glyphs.size() != 2) {
    return 48;
  }

  const cgpui::RasterizedGlyph rasterized = cgpui::rasterize_fallback_glyph(
      glyphs[1],
      cgpui::GlyphRasterizerOptions{.foreground_alpha = 220});
  if (!(rasterized.key == glyphs[1].key) ||
      rasterized.key.byte_length != 3 ||
      rasterized.advance != glyphs[1].device_advance ||
      rasterized.device_font_size != run.device_font_size) {
    return 49;
  }
  if (rasterized.bitmap.empty() || rasterized.bitmap.width != 10 ||
      rasterized.bitmap.height != 20 || rasterized.bitmap.stride != 10 ||
      rasterized.bitmap.byte_size() != 200) {
    return 50;
  }
  if (rasterized.bitmap.pixel(0, 0) != 0 ||
      rasterized.bitmap.pixel(1, 1) != 220 ||
      rasterized.bitmap.pixel(9, 19) != 0) {
    return 51;
  }
  if (rasterized.left_bearing != 0.0F || rasterized.top_bearing != 0.0F ||
      rasterized.baseline != 16.0F) {
    return 52;
  }

  const cgpui::RasterizedGlyph rerasterized = cgpui::rasterize_fallback_glyph(
      glyphs[1],
      cgpui::GlyphRasterizerOptions{.foreground_alpha = 220});
  return rerasterized.bitmap.alpha == rasterized.bitmap.alpha ? 0 : 53;
}

} // namespace

int main() {
  if (const int result = test_text_model_defaults_to_empty_buffer();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_inserts_utf8_at_cursor();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_moves_cursor_by_utf8_codepoints();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_moves_cursor_by_grapheme_clusters();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_model_backspace_deletes_previous_utf8_codepoint();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_delete_removes_next_utf8_codepoint();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_tracks_selection_range();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_applies_key_edit_actions();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_applies_word_edit_actions();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_navigates_multiline_lines();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_undo_redo_restores_edit_history();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_model_deletes_surrounding_text_on_utf8_boundaries();
      result != 0) {
    return result;
  }
  if (const int result = test_text_model_tracks_ime_composition();
      result != 0) {
    return result;
  }
  if (const int result = test_font_database_registers_and_resolves_faces();
      result != 0) {
    return result;
  }
  if (const int result = test_font_database_resolves_ordered_fallback_chain();
      result != 0) {
    return result;
  }
  if (const int result = test_fake_font_discovery_is_deterministic();
      result != 0) {
    return result;
  }
  if (const int result =
          test_platform_font_discovery_records_preserve_metadata();
      result != 0) {
    return result;
  }
  if (const int result = test_shape_text_produces_deterministic_fallback_glyphs();
      result != 0) {
    return result;
  }
  if (const int result = test_text_measurement_cache_reuses_same_text_tuple();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_hit_geometry_maps_points_to_offsets_and_selection();
      result != 0) {
    return result;
  }
  if (const int result =
          test_fallback_glyph_rasterizer_produces_deterministic_bitmap();
      result != 0) {
    return result;
  }
  return 0;
}

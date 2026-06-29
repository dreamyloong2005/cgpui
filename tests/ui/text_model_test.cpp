#include "cgpui/ui/text.hpp"

#include <string_view>

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
      model.selection().end != 3 || model.selection().collapsed) {
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
      model.selection().end != model.cursor()) {
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
  if (const int result = test_text_model_tracks_ime_composition();
      result != 0) {
    return result;
  }
  return 0;
}

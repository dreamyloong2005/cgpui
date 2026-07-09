#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

struct TextInputWorkflowSnapshot {
  std::size_t line_count = 0;
  std::size_t gamma_line = 0;
  std::size_t gamma_line_start = 0;
  std::size_t gamma_line_end = 0;
  std::size_t cursor_after_navigation = 0;
  cgpui::TextSelectionRange extended_line_selection;
  std::string selected_word;
  cgpui::TextEditHistoryStatus after_insert;
  cgpui::TextEditHistoryStatus after_undo;
  cgpui::TextEditHistoryStatus after_redo;
  cgpui::TextEditHistoryStatus after_backspace;
  cgpui::TextEditHistoryStatus after_delete_forward;
  cgpui::TextEditHistoryStatus after_branch_edit;
  bool undo_applied = false;
  bool redo_applied = false;
  bool backspace_applied = false;
  bool delete_forward_applied = false;
  bool had_composition = false;
  bool composition_cancelled = false;
};

class PublicTextInputWorkflowView final : public cgpui::View {
 public:
  PublicTextInputWorkflowView() {
    snapshot_ = run_public_workflow(input_model_);
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicTextInputWorkflowView>& context) override {
    return cgpui::into_element(
        cgpui::v_flex()
            .size(context.viewport_size)
            .padding(cgpui::edges(24.0F))
            .gap(10.0F)
            .background(cgpui::rgb(23, 29, 35))
            .child(cgpui::label("Public text input workflow")
                       .font_size(20.0F)
                       .foreground(cgpui::rgb(244, 247, 250))
                       .build())
            .child(cgpui::text_input(input_model_)
                       .key("phase-d-public-text-input-workflow")
                       .style(input_style())
                       .foreground(cgpui::rgb(246, 249, 252))
                       .font_size(14.0F)
                       .build())
            .child(cgpui::label(history_line())
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(194, 208, 219))
                       .build())
            .child(cgpui::label(navigation_line())
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(174, 193, 208))
                       .build())
            .child(cgpui::label(composition_line())
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(204, 216, 224))
                       .build()));
  }

 private:
  static cgpui::Style input_style() {
    return cgpui::Style{}
        .with_preferred_size(cgpui::Size{540.0F, 48.0F})
        .with_background_color(cgpui::rgb(48, 58, 68))
        .with_border_width(cgpui::edges(1.0F))
        .with_border_color(cgpui::rgb(88, 108, 128))
        .with_border_radius(cgpui::BorderRadii::all(6.0F))
        .with_padding(cgpui::edges(12.0F, 8.0F));
  }

  static TextInputWorkflowSnapshot run_public_workflow(cgpui::TextModel& model) {
    TextInputWorkflowSnapshot snapshot;
    const std::string_view text = model.text();
    const std::size_t gamma_offset = text.find("gamma");
    snapshot.line_count = model.line_count();
    snapshot.gamma_line = model.line_index_at(gamma_offset);
    snapshot.gamma_line_start = model.line_start_offset(gamma_offset);
    snapshot.gamma_line_end = model.line_end_offset(gamma_offset);

    model.set_selection(0, 5);
    snapshot.selected_word = model.selected_text();
    model.clear_selection();
    (void)model.move_cursor_line_start();
    (void)model.move_cursor_next_word();
    (void)model.move_cursor_next_line();
    snapshot.cursor_after_navigation = model.cursor();
    (void)model.apply_edit_action(cgpui::TextEditAction::extend_line_end);
    snapshot.extended_line_selection = model.selection();
    model.clear_selection();

    model.mark_edit_history_clean();
    model.insert_text("!", cgpui::TextInsertHistoryPolicy::separate_edit);
    snapshot.after_insert = model.edit_history_status();
    snapshot.undo_applied = model.undo();
    snapshot.after_undo = model.edit_history_status();
    snapshot.redo_applied = model.redo();
    snapshot.after_redo = model.edit_history_status();

    snapshot.backspace_applied = model.backspace();
    snapshot.after_backspace = model.edit_history_status();
    (void)model.move_cursor_line_start();
    snapshot.delete_forward_applied = model.delete_forward();
    snapshot.after_delete_forward = model.edit_history_status();

    (void)model.undo();
    model.insert_text(" branch", cgpui::TextInsertHistoryPolicy::separate_edit);
    snapshot.after_branch_edit = model.edit_history_status();

    model.set_composition_text(" pending");
    snapshot.had_composition = model.has_composition();
    model.cancel_composition();
    snapshot.composition_cancelled = !model.has_composition();
    return snapshot;
  }

  static bool branch_redo_invalidated(
      const cgpui::TextEditHistoryStatus& status) {
    return status.last_redo_invalidation.reason ==
           cgpui::TextEditHistoryRedoInvalidationReason::branch_edit;
  }

  static bool undo_was_recorded(
      const cgpui::TextEditHistoryStatus& status) {
    return status.last_transaction.kind ==
           cgpui::TextEditHistoryTransactionKind::undo_applied;
  }

  [[nodiscard]] std::string history_line() const {
    return "undo=" + yes_no(snapshot_.undo_applied) +
           " redo=" + yes_no(snapshot_.redo_applied) +
           " undo_kind=" + yes_no(undo_was_recorded(snapshot_.after_undo)) +
           " branch_invalidated=" +
           yes_no(branch_redo_invalidated(snapshot_.after_branch_edit)) +
           " undo_depth=" +
           std::to_string(snapshot_.after_branch_edit.undo_depth);
  }

  [[nodiscard]] std::string navigation_line() const {
    return "lines=" + std::to_string(snapshot_.line_count) +
           " gamma_line=" + std::to_string(snapshot_.gamma_line) +
           " gamma_range=" + std::to_string(snapshot_.gamma_line_start) +
           "-" + std::to_string(snapshot_.gamma_line_end) +
           " selected=\"" + snapshot_.selected_word + "\" cursor=" +
           std::to_string(snapshot_.cursor_after_navigation) +
           " extend=" +
           std::to_string(snapshot_.extended_line_selection.start) + "-" +
           std::to_string(snapshot_.extended_line_selection.end);
  }

  [[nodiscard]] std::string composition_line() const {
    return "backspace=" + yes_no(snapshot_.backspace_applied) +
           " delete_forward=" + yes_no(snapshot_.delete_forward_applied) +
           " composition_seen=" + yes_no(snapshot_.had_composition) +
           " composition_cancelled=" +
           yes_no(snapshot_.composition_cancelled) + " insert_revision=" +
           std::to_string(snapshot_.after_insert.revision) +
           " redo_revision=" + std::to_string(snapshot_.after_redo.revision) +
           " delete_revision=" +
           std::to_string(snapshot_.after_delete_forward.revision);
  }

  static std::string yes_no(bool value) {
    return value ? "yes" : "no";
  }

  cgpui::TextModel input_model_{"alpha beta\ngamma delta"};
  TextInputWorkflowSnapshot snapshot_;
};

using TextModelRef = cgpui::TextModel&;

static_assert(cgpui::Render<PublicTextInputWorkflowView>);
static_assert(
    std::is_same_v<decltype(std::declval<TextModelRef>()
                                .mark_edit_history_clean()),
                   void>);
static_assert(
    std::is_same_v<decltype(std::declval<TextModelRef>()
                                .edit_history_status()),
                   cgpui::TextEditHistoryStatus>);
static_assert(
    std::is_same_v<decltype(std::declval<TextModelRef>().undo()), bool>);
static_assert(
    std::is_same_v<decltype(std::declval<TextModelRef>().redo()), bool>);
static_assert(
    std::is_same_v<decltype(std::declval<TextModelRef>().backspace()), bool>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .delete_forward()),
                             bool>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .move_cursor_next_word()),
                             bool>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .move_cursor_next_line()),
                             bool>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .apply_edit_action(
                                              cgpui::TextEditAction::extend_line_end)),
                             bool>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .line_count()),
                             std::size_t>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .line_index_at(0)),
                             std::size_t>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .line_start_offset(0)),
                             std::size_t>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .line_end_offset(0)),
                             std::size_t>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .selected_text()),
                             std::string>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .set_composition_text(
                                              std::declval<std::string_view>())),
                             void>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .cancel_composition()),
                             void>);
static_assert(std::is_same_v<decltype(std::declval<TextModelRef>()
                                          .has_composition()),
                             bool>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Text Input Workflow")
                       .size(660.0F, 360.0F)
                       .decorations(true)
                       .resizable(true)
                       .transparent(false)
                       .titlebar_visible(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_TEXT_INPUT_WORKFLOW") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicTextInputWorkflowView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

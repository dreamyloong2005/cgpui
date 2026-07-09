#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

struct SelectWordAction {
  static constexpr std::string_view name = "text_input_examples.select_word";
};

struct CommitCompositionAction {
  static constexpr std::string_view name =
      "text_input_examples.commit_composition";
};

struct TextInputExampleSnapshot {
  cgpui::TextSelectionRange selection;
  cgpui::TextSelectionRange word_range;
  cgpui::TextSelectionRange line_range;
  cgpui::TextEditHistoryStatus history;
  cgpui::ImeTextInputPlacement placement;
  cgpui::ImeComposition composition;
  cgpui::ImeDeleteSurroundingText delete_request;
};

class PublicTextInputExamplesView final : public cgpui::View {
 public:
  PublicTextInputExamplesView() {
    prepare_text_input_model();
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicTextInputExamplesView>& context) override {
    install_actions(context);

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .bg(cgpui::rgb(19, 24, 29))
            .p(24.0F)
            .child(cgpui::v_flex()
                       .size_pct(100.0F, 100.0F)
                       .gap(12.0F)
                       .p(18.0F)
                       .rounded(8.0F)
                       .bg(cgpui::rgb(39, 47, 55))
                       .child(cgpui::label("Text input public examples")
                                  .font_size(20.0F)
                                  .foreground(cgpui::rgb(243, 246, 249))
                                  .build())
                       .child(cgpui::label(status_line())
                                  .font_size(12.0F)
                                  .foreground(cgpui::rgb(182, 195, 207))
                                  .build())
                       .child(cgpui::text_input(input_model_)
                                  .key("phase-d-public-text-input")
                                  .style(input_style())
                                  .foreground(cgpui::rgb(246, 249, 252))
                                  .font_size(14.0F)
                                  .build())
                       .child(cgpui::h_flex()
                                  .gap(10.0F)
                                  .child(cgpui::button(SelectWordAction::name)
                                             .label("Select word")
                                             .build())
                                  .child(cgpui::button(
                                             CommitCompositionAction::name)
                                             .label("Commit composition")
                                             .build()))
                       .child(cgpui::label(ime_line())
                                  .font_size(12.0F)
                                  .foreground(cgpui::rgb(200, 211, 221))
                                  .build())));
  }

 private:
  static cgpui::Style input_style() {
    return cgpui::Style{}
        .with_preferred_size(cgpui::Size{520.0F, 44.0F})
        .with_background_color(cgpui::rgb(50, 60, 70))
        .with_border_width(cgpui::edges(1.0F))
        .with_border_color(cgpui::rgb(92, 111, 130))
        .with_border_radius(cgpui::BorderRadii::all(6.0F))
        .with_padding(cgpui::edges(12.0F, 8.0F));
  }

  static cgpui::ImeComposition preview_composition() {
    cgpui::ImeComposition composition;
    composition.phase = cgpui::ImeCompositionPhase::update;
    composition.text = "preedit";
    composition.serial = 44;
    composition.preedit_cursor_begin = 0;
    composition.preedit_cursor_end = 7;
    (void)cgpui::append_ime_default_preedit_style(composition);
    return composition;
  }

  static cgpui::ImeDeleteSurroundingText preview_delete_request() {
    cgpui::ImeDeleteSurroundingText request;
    request.before_length = 1;
    request.after_length = 0;
    request.serial = 44;
    return request;
  }

  static cgpui::ImeTextInputPlacement placement_for(
      const cgpui::TextModel& model) {
    cgpui::ImeTextInputPlacement placement;
    placement.rect =
        cgpui::Rect{.origin = {24.0F, 116.0F},
                    .size = {.width = 520.0F, .height = 44.0F}};
    placement.candidate_rect =
        cgpui::Rect{.origin = {24.0F, 164.0F},
                    .size = {.width = 240.0F, .height = 32.0F}};
    placement.byte_offset = model.cursor();
    placement.surrounding_text = std::string(model.text());
    placement.selection_anchor = model.selection_anchor();
    placement.content_hint = 1;
    placement.content_purpose = 1;
    return placement;
  }

  void install_actions(
      cgpui::Context<PublicTextInputExamplesView>& context) const {
    context.register_command_palette_entry<SelectWordAction>(
        cgpui::CommandPaletteEntry{
            .title = "Select text input word",
            .group = "Text Input Examples",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-alt-w",
        });
    context.register_command_palette_entry<CommitCompositionAction>(
        cgpui::CommandPaletteEntry{
            .title = "Commit text input composition",
            .group = "Text Input Examples",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-alt-c",
        });

    (void)context.bind_key(
        "ctrl-alt-w",
        std::string(SelectWordAction::name),
        cgpui::KeyBindingContext::window());
    (void)context.bind_key(
        "ctrl-alt-c",
        std::string(CommitCompositionAction::name),
        cgpui::KeyBindingContext::window());
  }

  void prepare_text_input_model() {
    input_model_.set_selection(0, 5);
    snapshot_.selection = input_model_.selection();
    snapshot_.word_range = input_model_.word_selection_range_at(6);
    snapshot_.line_range = input_model_.line_selection_range_at(0);
    selected_text_ = input_model_.selected_text();

    input_model_.clear_selection();
    (void)input_model_.move_cursor_line_end();
    input_model_.insert_text(
        " edited",
        cgpui::TextInsertHistoryPolicy::merge_adjacent_typing);
    input_model_.set_composition_text(" pending");
    input_model_.commit_composition();
    input_model_.insert_text(
        " grouped",
        cgpui::TextInsertHistoryPolicy::composition_commit);
    (void)input_model_.delete_surrounding_text(1, 0);
    (void)input_model_.apply_edit_action(cgpui::TextEditAction::move_next_word);

    snapshot_.history = input_model_.edit_history_status();
    snapshot_.placement = placement_for(input_model_);
    snapshot_.composition = preview_composition();
    snapshot_.delete_request = preview_delete_request();
  }

  [[nodiscard]] std::string status_line() const {
    return "selected=\"" + selected_text_ + "\" word=" +
           std::to_string(snapshot_.word_range.start) + "-" +
           std::to_string(snapshot_.word_range.end) + " line=" +
           std::to_string(snapshot_.line_range.start) + "-" +
           std::to_string(snapshot_.line_range.end) + " undo=" +
           std::to_string(snapshot_.history.undo_depth);
  }

  [[nodiscard]] std::string ime_line() const {
    const std::size_t style_count = snapshot_.composition.preedit_style_count;
    return "ime byte=" + std::to_string(snapshot_.placement.byte_offset) +
           " candidate=" +
           (snapshot_.placement.candidate_rect.has_value() ? "yes" : "no") +
           " preedit_styles=" + std::to_string(style_count) +
           " delete_before=" +
           std::to_string(snapshot_.delete_request.before_length);
  }

  cgpui::TextModel input_model_{"alpha beta\ngamma delta"};
  TextInputExampleSnapshot snapshot_;
  std::string selected_text_;
};

using TextInputModelRef = cgpui::TextModel&;
using TextInputContextRef = cgpui::Context<PublicTextInputExamplesView>&;

static_assert(cgpui::Action<SelectWordAction>);
static_assert(cgpui::Action<CommitCompositionAction>);
static_assert(cgpui::Render<PublicTextInputExamplesView>);
static_assert(
    std::is_same_v<decltype(std::declval<TextInputModelRef>().insert_text(
                       std::declval<std::string_view>(),
                       cgpui::TextInsertHistoryPolicy::merge_adjacent_typing)),
                   void>);
static_assert(
    std::is_same_v<decltype(std::declval<TextInputModelRef>()
                                .edit_history_status()),
                   cgpui::TextEditHistoryStatus>);
static_assert(
    std::is_same_v<decltype(std::declval<TextInputModelRef>()
                                .word_selection_range_at(0)),
                   cgpui::TextSelectionRange>);
static_assert(
    std::is_same_v<decltype(std::declval<TextInputModelRef>()
                                .delete_surrounding_text(1, 0)),
                   bool>);
static_assert(
    std::is_same_v<decltype(std::declval<TextInputContextRef>()
                                .bind_key(
                                    std::declval<std::string_view>(),
                                    std::declval<std::string>(),
                                    cgpui::KeyBindingContext::window())),
                   bool>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Text Input Examples")
                       .size(680.0F, 380.0F)
                       .decorations(true)
                       .resizable(true)
                       .transparent(false)
                       .titlebar_visible(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_TEXT_INPUT_EXAMPLES") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicTextInputExamplesView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

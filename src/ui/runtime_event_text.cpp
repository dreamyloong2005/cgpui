#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::apply_text_input_for_event(const PlatformEvent& event) {
  if (!keyboard_focus_element_owner_.has_value()) {
    return;
  }

  TextModel* model = focused_text_model();
  if (model == nullptr) {
    return;
  }

  if (const auto* text = std::get_if<TextInput>(&event); text != nullptr) {
    model->insert_text(text->text);
    return;
  }

  if (const auto* composition = std::get_if<ImeComposition>(&event);
      composition != nullptr) {
    switch (composition->phase) {
      case ImeCompositionPhase::update:
        model->set_composition_text(composition->text);
        break;
      case ImeCompositionPhase::commit:
        model->set_composition_text(composition->text);
        model->commit_composition();
        break;
      case ImeCompositionPhase::cancel:
        model->cancel_composition();
        break;
    }
    return;
  }

  const auto* delete_surrounding =
      std::get_if<ImeDeleteSurroundingText>(&event);
  if (delete_surrounding == nullptr) {
    return;
  }

  (void)model->delete_surrounding_text(
      delete_surrounding->before_length,
      delete_surrounding->after_length);
}

} // namespace cgpui

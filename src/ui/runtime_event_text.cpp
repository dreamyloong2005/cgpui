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
    const char* operation = "composition-update";
    bool cancelled = false;
    switch (composition->phase) {
      case ImeCompositionPhase::update:
        model->set_composition_text(composition->text);
        break;
      case ImeCompositionPhase::commit:
        operation = "composition-commit";
        model->set_composition_text(composition->text);
        model->commit_composition();
        break;
      case ImeCompositionPhase::cancel:
        operation = "composition-cancel";
        cancelled = true;
        model->cancel_composition();
        break;
    }
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::ime,
        .event_kind = event_kind_for(event),
        .backend = "runtime",
        .operation = operation,
        .supported = true,
        .succeeded = !cancelled,
        .cancelled = cancelled,
        .value_count = composition->text.size(),
    });
    return;
  }

  const auto* delete_surrounding =
      std::get_if<ImeDeleteSurroundingText>(&event);
  if (delete_surrounding == nullptr) {
    return;
  }

  const bool deleted = model->delete_surrounding_text(
      delete_surrounding->before_length, delete_surrounding->after_length);
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::ime,
      .event_kind = event_kind_for(event),
      .backend = "runtime",
      .operation = "delete-surrounding",
      .supported = true,
      .succeeded = deleted,
      .value_count =
          static_cast<std::size_t>(delete_surrounding->before_length) +
          static_cast<std::size_t>(delete_surrounding->after_length),
  });
}

} // namespace cgpui

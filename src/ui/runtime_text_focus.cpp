#include "ui_internal.hpp"

namespace cgpui {

TextModel* WindowRuntime::focused_text_model() {
  if (!state_->input.keyboard_focus_element_owner_.has_value()) {
    return nullptr;
  }

  const auto model = state_->text.text_models_.find(state_->input.keyboard_focus_element_owner_->value);
  if (model != state_->text.text_models_.end()) {
    return model->second;
  }

  auto* input = dynamic_cast<TextInputElement*>(
      routed_element(*state_->input.keyboard_focus_element_owner_));
  return input == nullptr ? nullptr : input->model();
}

const TextModel* WindowRuntime::focused_text_model() const {
  if (!state_->input.keyboard_focus_element_owner_.has_value()) {
    return nullptr;
  }

  const auto model = state_->text.text_models_.find(state_->input.keyboard_focus_element_owner_->value);
  if (model != state_->text.text_models_.end()) {
    return model->second;
  }

  const auto* input = dynamic_cast<const TextInputElement*>(
      routed_element(*state_->input.keyboard_focus_element_owner_));
  return input == nullptr ? nullptr : input->model();
}

std::optional<ImeCandidateRect> WindowRuntime::focused_text_ime_rect() const {
  const TextModel* model = focused_text_model();
  if (!state_->input.keyboard_focus_element_owner_.has_value() || model == nullptr) {
    return {};
  }

  const auto* text_element = dynamic_cast<const TextElement*>(
      routed_element(*state_->input.keyboard_focus_element_owner_));
  if (text_element == nullptr) {
    return {};
  }

  const std::size_t byte_offset = model->cursor();
  const std::optional<Rect> rect = text_element->caret_rect(state_->windows.scale_);
  if (!rect.has_value()) {
    return {};
  }
  return ImeCandidateRect{
      .element_id = *state_->input.keyboard_focus_element_owner_,
      .rect = *rect,
      .byte_offset = byte_offset,
      .composition_rect = text_element->composition_start_rect(state_->windows.scale_),
  };
}

} // namespace cgpui

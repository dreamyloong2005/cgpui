#include "ui_internal.hpp"

namespace cgpui {

TextInputElement* WindowRuntime::routed_text_input(ElementId element_id) {
  return dynamic_cast<TextInputElement*>(routed_element(element_id));
}

const TextInputElement* WindowRuntime::routed_text_input(
    ElementId element_id) const {
  return dynamic_cast<const TextInputElement*>(routed_element(element_id));
}

void WindowRuntime::bind_text_model(ElementId element_id, TextModel* model) {
  if (element_id.value == 0) {
    return;
  }
  if (model == nullptr) {
    text_models_.erase(element_id.value);
    return;
  }
  text_models_[element_id.value] = model;
}

} // namespace cgpui

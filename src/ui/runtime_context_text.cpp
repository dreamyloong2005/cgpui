#include "ui_internal.hpp"

#include <utility>

namespace cgpui {

void WindowRuntimeContext::bind_text_model(
    ElementId element_id,
    TextModel* model) const {
  runtime.bind_text_model(element_id, model);
}

bool WindowRuntimeContext::mutate_focused_text_model(
    FocusedTextModelMutation mutation) const {
  if (!mutation) {
    return false;
  }

  TextModel* model = runtime.focused_text_model();
  if (model == nullptr) {
    return false;
  }

  mutation(*model);
  return true;
}

std::optional<ImeCandidateRect> WindowRuntimeContext::focused_text_ime_rect()
    const {
  return runtime.focused_text_ime_rect();
}

AccessibilityTreeSnapshot WindowRuntimeContext::accessibility_snapshot()
    const {
  return runtime.accessibility_snapshot();
}

bool WindowRuntimeContext::paste_clipboard_text() const {
  return runtime.paste_clipboard_text();
}

bool WindowRuntimeContext::copy_selection_to_clipboard() const {
  return runtime.copy_selection_to_clipboard();
}

bool WindowRuntimeContext::cut_selection_to_clipboard() const {
  return runtime.cut_selection_to_clipboard();
}

} // namespace cgpui

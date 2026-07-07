#include "cgpui/prelude.hpp"

#include <string>

int test_button_label_convenience_builds_accessible_button() {
  cgpui::AnyElement element =
      cgpui::button("dialog.confirm")
          .label("Confirm")
          .style(cgpui::Style{}.with_padding(cgpui::edges(4.0F)))
          .key("confirm-button")
          .build();

  const auto* button = dynamic_cast<const cgpui::ButtonElement*>(element.get());
  if (button == nullptr || button->action_name() != "dialog.confirm") {
    return 1;
  }
  if (!button->focusable() || !button->key().has_value() ||
      button->key()->value != "confirm-button") {
    return 2;
  }
  const auto* label = dynamic_cast<const cgpui::LabelElement*>(button->child());
  if (label == nullptr || label->text() != "Confirm") {
    return 3;
  }
  if (button->accessibility_name() != "Confirm") {
    return 4;
  }

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  const std::optional<cgpui::Rect> button_bounds = button->layout_bounds();
  const std::optional<cgpui::Rect> label_bounds = label->layout_bounds();
  if (!button_bounds.has_value() || !label_bounds.has_value()) {
    return 5;
  }
  if (output.size.width != 64.0F || output.size.height != 24.0F ||
      label_bounds->origin.x != 4.0F || label_bounds->origin.y != 4.0F ||
      label_bounds->size.width != 56.0F || label_bounds->size.height != 16.0F) {
    return 6;
  }
  return 0;
}

int test_text_input_widget_builder_keeps_text_model_boundary() {
  cgpui::TextModel model("typed");
  cgpui::AnyElement element =
      cgpui::text_input(model).key("primary-input").disabled().build();

  const auto* input =
      dynamic_cast<const cgpui::TextInputElement*>(element.get());
  if (input == nullptr || input->model() != &model ||
      input->accessibility_role() != cgpui::AccessibilityRole::text_input) {
    return 10;
  }
  if (input->enabled() || !input->key().has_value() ||
      input->key()->value != "primary-input") {
    return 11;
  }
  if (!input->focusable() || input->text() != "typed") {
    return 12;
  }
  return 0;
}

int main() {
  if (const int result =
          test_button_label_convenience_builds_accessible_button();
      result != 0) {
    return result;
  }
  return test_text_input_widget_builder_keeps_text_model_boundary();
}

#include "cgpui/ui/element_button_nodes.hpp"
#include "cgpui/ui/element_choice_nodes.hpp"
#include "cgpui/ui/element_slider_nodes.hpp"
#include "cgpui/ui/element_text_nodes.hpp"

#include <cmath>

int main() {
  const cgpui::ElementEventContext context{
      .target_element_id = cgpui::ElementId{1},
  };
  int invokes = 0;
  cgpui::ButtonElement button(
      "invoke", {}, [&](const cgpui::ElementEventContext&) {
        invokes += 1;
        return cgpui::EventResult::consumed_event();
      });
  if (!button.accessibility_patterns().invokable ||
      !button.handle_accessibility_action(
                 {.kind = cgpui::AccessibilityActionKind::invoke}, context)
           .consumed ||
      invokes != 1) return 1;

  cgpui::ToggleControlElement toggle(
      cgpui::ToggleControlKind::checkbox, "toggle", {}, false);
  if (toggle.accessibility_patterns().toggled != false ||
      !toggle.handle_accessibility_action(
                 {.kind = cgpui::AccessibilityActionKind::toggle}, context)
           .consumed ||
      !toggle.checked()) return 2;

  float changed = 0.0F;
  cgpui::SliderElement slider(
      "range", {}, 0.0F, 100.0F, 25.0F, 5.0F,
      [&](float value, const cgpui::ElementEventContext&) {
        changed = value;
        return cgpui::EventResult::consumed_event();
      });
  const auto range = slider.accessibility_patterns().range;
  if (!range.has_value() || range->minimum != 0.0 || range->maximum != 100.0 ||
      range->small_change != 5.0 ||
      !slider.handle_accessibility_action(
                 {.kind = cgpui::AccessibilityActionKind::set_range_value,
                  .numeric_value = 75.0},
                 context)
           .consumed ||
      std::abs(slider.value() - 75.0F) > 0.001F ||
      std::abs(changed - 75.0F) > 0.001F) return 3;

  cgpui::TextModel model{"before"};
  cgpui::TextInputElement input{model};
  if (!input.accessibility_patterns().value_settable ||
      !input.handle_accessibility_action(
                {.kind = cgpui::AccessibilityActionKind::set_value,
                 .value = "after"},
                context)
           .consumed ||
      model.text() != "after") return 4;
  return 0;
}

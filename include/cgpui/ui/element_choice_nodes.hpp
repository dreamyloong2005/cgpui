#pragma once

#include "cgpui/ui/element_core.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace cgpui {

enum class ToggleControlKind {
  checkbox,
  radio,
  toggle_switch,
};

class ToggleControlElement : public Element {
 public:
  ToggleControlElement(
      ToggleControlKind kind,
      std::string action_name,
      StyleState style_state,
      bool checked,
      ClickHandler click_handler = {},
      std::unique_ptr<Element> child = {});

  [[nodiscard]] ToggleControlKind kind() const;
  [[nodiscard]] std::string_view action_name() const;
  [[nodiscard]] const StyleState& style_state() const;
  [[nodiscard]] bool checked() const;
  [[nodiscard]] bool selected() const;
  [[nodiscard]] bool on() const;
  void set_checked(bool checked);
  [[nodiscard]] Element* child();
  [[nodiscard]] const Element* child() const;
  [[nodiscard]] bool focusable() const override;
  [[nodiscard]] AccessibilityRole accessibility_role() const override;
  [[nodiscard]] std::string accessibility_name() const override;
  [[nodiscard]] std::string accessibility_value() const override;
  [[nodiscard]] AccessibilityPatternState accessibility_patterns()
      const override;
  [[nodiscard]] EventResult handle_accessibility_action(
      const AccessibilityActionRequested& action,
      const ElementEventContext& context) override;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  void inherit_text_style(const Style& style) override;
  [[nodiscard]] ElementId hit_test(Point point) const override;
  void paint(PaintList& paint_list) const override;
  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override;
  [[nodiscard]] int z_index() const override;
  [[nodiscard]] int layer() const override;

 private:
  ToggleControlKind kind_;
  std::string action_name_;
  StyleState style_state_;
  bool checked_ = false;
  ClickHandler click_handler_;
  Style inherited_text_style_;
  std::unique_ptr<Element> child_;
};

} // namespace cgpui

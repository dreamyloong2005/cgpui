#pragma once

#include "cgpui/ui/element_core.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace cgpui {

class ButtonElement : public Element {
 public:
  ButtonElement(
      std::string action_name,
      StyleState style_state,
      ClickHandler click_handler = {},
      std::unique_ptr<Element> child = {});

  [[nodiscard]] std::string_view action_name() const;
  [[nodiscard]] const StyleState& style_state() const;
  [[nodiscard]] Element* child();
  [[nodiscard]] const Element* child() const;
  [[nodiscard]] bool focusable() const override;
  [[nodiscard]] AccessibilityRole accessibility_role() const override;
  [[nodiscard]] std::string accessibility_name() const override;
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
  std::string action_name_;
  StyleState style_state_;
  ClickHandler click_handler_;
  Style inherited_text_style_;
  std::unique_ptr<Element> child_;
};

} // namespace cgpui

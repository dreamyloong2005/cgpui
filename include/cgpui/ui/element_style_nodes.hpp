#pragma once

#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/style_cascade.hpp"

#include <memory>

namespace cgpui {

class StyledElement : public Element {
 public:
  explicit StyledElement(Style style, std::unique_ptr<Element> child = {});
  explicit StyledElement(
      StyleState style_state,
      std::unique_ptr<Element> child = {});

  [[nodiscard]] const Style& style() const;
  [[nodiscard]] const StyleState& style_state() const;
  [[nodiscard]] const StyleClasses& style_classes() const;
  [[nodiscard]] const StyleOverlay& inline_style() const;
  void set_style_classes(StyleClasses classes);
  void set_inline_style(StyleOverlay style);
  [[nodiscard]] Style resolved_style(
      const StyleCascade& cascade,
      StyleStateFlags flags) const;
  [[nodiscard]] Element* child();
  [[nodiscard]] const Element* child() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  void paint(PaintList& paint_list) const override;
  [[nodiscard]] ElementId hit_test(Point point) const override;
  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override;
  [[nodiscard]] int z_index() const override;
  [[nodiscard]] int layer() const override;

 private:
  StyleState style_state_;
  StyleClasses style_classes_;
  StyleOverlay inline_style_;
  std::unique_ptr<Element> child_;
};

} // namespace cgpui

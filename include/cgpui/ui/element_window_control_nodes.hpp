#pragma once

#include "cgpui/core/window_title_bar.hpp"
#include "cgpui/ui/element_core.hpp"

#include <memory>

namespace cgpui {

// Declares its laid-out bounds as a client title bar region while painting
// (GPUI's `window_control_area`). Layout, hit testing and events go to the
// child unchanged.
class WindowControlAreaElement : public Element {
 public:
  WindowControlAreaElement(
      std::unique_ptr<Element> child, WindowControlArea area);

  [[nodiscard]] WindowControlArea area() const;
  [[nodiscard]] Element* child();
  [[nodiscard]] const Element* child() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  void inherit_text_style(const Style& style) override;
  [[nodiscard]] ElementId hit_test(Point point) const override;
  void paint(PaintList& paint_list) const override;
  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override;

 protected:
  void offset_layout_children(Point delta) const override;

 private:
  std::unique_ptr<Element> child_;
  WindowControlArea area_;
};

} // namespace cgpui

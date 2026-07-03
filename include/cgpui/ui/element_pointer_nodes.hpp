#pragma once

#include "cgpui/ui/element_core.hpp"

#include <memory>

namespace cgpui {

class ClickElement : public Element {
 public:
  ClickElement(std::unique_ptr<Element> child, ClickHandler handler);

  [[nodiscard]] Element* child();
  [[nodiscard]] const Element* child() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  [[nodiscard]] ElementId hit_test(Point point) const override;
  void paint(PaintList& paint_list) const override;
  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override;

 private:
  std::unique_ptr<Element> child_;
  ClickHandler handler_;
};

class PointerElement : public Element {
 public:
  PointerElement(
      std::unique_ptr<Element> child,
      PointerButtonHandler down_handler,
      PointerButtonHandler up_handler,
      PointerMoveHandler move_handler);

  [[nodiscard]] Element* child();
  [[nodiscard]] const Element* child() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  [[nodiscard]] ElementId hit_test(Point point) const override;
  void paint(PaintList& paint_list) const override;
  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override;

 private:
  std::unique_ptr<Element> child_;
  PointerButtonHandler down_handler_;
  PointerButtonHandler up_handler_;
  PointerMoveHandler move_handler_;
};

} // namespace cgpui

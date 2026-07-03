#pragma once

#include "cgpui/ui/element_core.hpp"

#include <memory>

namespace cgpui {

class ChildViewElement : public Element {
 public:
  ChildViewElement(ViewId view_id, Size placeholder_size);

  [[nodiscard]] ViewId view_id() const;
  [[nodiscard]] Size placeholder_size() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;

 private:
  ViewId view_id_;
  Size placeholder_size_;
};

class FocusableElement : public Element {
 public:
  explicit FocusableElement(std::unique_ptr<Element> child);

  [[nodiscard]] Element* child();
  [[nodiscard]] const Element* child() const;
  [[nodiscard]] bool focusable() const override;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  [[nodiscard]] ElementId hit_test(Point point) const override;
  void paint(PaintList& paint_list) const override;
  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override;

 private:
  std::unique_ptr<Element> child_;
};

class KeyElement : public Element {
 public:
  KeyElement(std::unique_ptr<Element> child, KeyHandler handler);

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
  KeyHandler handler_;
};

} // namespace cgpui

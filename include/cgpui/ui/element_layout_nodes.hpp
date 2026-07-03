#pragma once

#include "cgpui/ui/element_core.hpp"

#include <memory>
#include <span>
#include <vector>

namespace cgpui {

class FixedSizeElement : public Element {
 public:
  explicit FixedSizeElement(Size preferred_size);

  [[nodiscard]] Size preferred_size() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;

 private:
  Size preferred_size_;
};

class VerticalStackElement : public Element {
 public:
  [[nodiscard]] float gap() const;
  void set_gap(float gap);
  void append_child(std::unique_ptr<Element> child);
  [[nodiscard]] std::span<const std::unique_ptr<Element>> children() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  [[nodiscard]] ElementId hit_test(Point point) const override;

 private:
  float gap_ = 0.0F;
  std::vector<std::unique_ptr<Element>> children_;
};

enum class FlexDirection {
  row,
  column,
};

class FlexElement : public Element {
 public:
  explicit FlexElement(FlexDirection direction);

  [[nodiscard]] FlexDirection direction() const;
  [[nodiscard]] float gap() const;
  void set_gap(float gap);
  [[nodiscard]] AlignItems align_items() const;
  void set_align_items(AlignItems align_items);
  [[nodiscard]] JustifyContent justify_content() const;
  void set_justify_content(JustifyContent justify_content);
  void append_child(std::unique_ptr<Element> child);
  [[nodiscard]] std::span<const std::unique_ptr<Element>> children() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  [[nodiscard]] ElementId hit_test(Point point) const override;

 private:
  FlexDirection direction_;
  float gap_ = 0.0F;
  AlignItems align_items_ = AlignItems::start;
  JustifyContent justify_content_ = JustifyContent::start;
  std::vector<std::unique_ptr<Element>> children_;
};

} // namespace cgpui

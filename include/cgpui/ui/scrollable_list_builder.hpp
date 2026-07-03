#pragma once

#include "cgpui/ui/element_builder_core.hpp"
#include "cgpui/ui/element_scroll_nodes.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {

class ScrollableListBuilder {
 public:
  explicit ScrollableListBuilder(ScrollState& state);

  [[nodiscard]] ScrollableListBuilder style(Style style) &&;
  [[nodiscard]] ScrollableListBuilder size(Size size) &&;
  [[nodiscard]] ScrollableListBuilder size(float width, float height) &&;
  [[nodiscard]] ScrollableListBuilder gap(float value) &&;
  [[nodiscard]] ScrollableListBuilder key(ElementKey key) &&;
  [[nodiscard]] ScrollableListBuilder key(std::string_view value) &&;
  [[nodiscard]] ScrollableListBuilder enabled(bool value) &&;
  [[nodiscard]] ScrollableListBuilder disabled() &&;
  [[nodiscard]] ScrollableListBuilder item(ElementKey key, AnyElement child) &&;
  [[nodiscard]] ScrollableListBuilder item(
      std::string_view key,
      AnyElement child) &&;
  [[nodiscard]] ScrollableListBuilder item(
      std::string_view key,
      ElementBuilder child) &&;

  template <typename T>
    requires std::derived_from<T, Element> && (!std::same_as<T, Element>)
  [[nodiscard]] ScrollableListBuilder item(
      std::string_view key,
      std::unique_ptr<T> child) && {
    return std::move(*this).item(
        key,
        std::unique_ptr<Element>(std::move(child)));
  }

  [[nodiscard]] AnyElement build() &&;

 private:
  ScrollState* state_ = nullptr;
  Style style_;
  float gap_ = 0.0F;
  std::optional<ElementKey> key_;
  bool enabled_ = true;
  std::vector<AnyElement> items_;
};

[[nodiscard]] ScrollableListBuilder scrollable_list(ScrollState& state);
[[nodiscard]] AnyElement scroll(ScrollState& state, AnyElement child);
[[nodiscard]] AnyElement scroll(ScrollState& state, ElementBuilder child);

} // namespace cgpui

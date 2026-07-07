#pragma once

#include "cgpui/ui/element_builder_core.hpp"
#include "cgpui/ui/element_item_nodes.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace cgpui {

class ItemBuilder {
 public:
  ItemBuilder(ItemKind kind, std::string action_name);

  [[nodiscard]] ItemBuilder style(Style style) &&;
  [[nodiscard]] ItemBuilder hover_style(StyleOverlay overlay) &&;
  [[nodiscard]] ItemBuilder focus_style(StyleOverlay overlay) &&;
  [[nodiscard]] ItemBuilder active_style(StyleOverlay overlay) &&;
  [[nodiscard]] ItemBuilder disabled_style(StyleOverlay overlay) &&;
  [[nodiscard]] ItemBuilder label(std::string_view text) &&;
  [[nodiscard]] ItemBuilder child(std::unique_ptr<Element> child) &&;
  [[nodiscard]] ItemBuilder child(ElementBuilder child) &&;

  template <typename T>
    requires std::derived_from<T, Element> && (!std::same_as<T, Element>)
  [[nodiscard]] ItemBuilder child(std::unique_ptr<T> child) && {
    return std::move(*this).child(std::unique_ptr<Element>(std::move(child)));
  }

  [[nodiscard]] ItemBuilder selected(bool value) &&;
  [[nodiscard]] ItemBuilder on_click(ClickHandler handler) &&;
  [[nodiscard]] ItemBuilder enabled(bool value) &&;
  [[nodiscard]] ItemBuilder disabled() &&;
  [[nodiscard]] ItemBuilder key(ElementKey key) &&;
  [[nodiscard]] ItemBuilder key(std::string_view value) &&;
  [[nodiscard]] AnyElement build() &&;

 private:
  ItemKind kind_;
  std::string action_name_;
  StyleState style_state_;
  ClickHandler click_handler_;
  std::unique_ptr<Element> child_;
  std::optional<ElementKey> key_;
  bool selected_ = false;
  bool enabled_ = true;
};

[[nodiscard]] ItemBuilder list_item(std::string_view action_name);
[[nodiscard]] ItemBuilder menu_item(std::string_view action_name);

} // namespace cgpui

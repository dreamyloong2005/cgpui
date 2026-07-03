#pragma once

#include "cgpui/ui/element_builder_core.hpp"
#include "cgpui/ui/element_button_nodes.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace cgpui {

class ButtonBuilder {
 public:
  explicit ButtonBuilder(std::string action_name);

  [[nodiscard]] ButtonBuilder style(Style style) &&;
  [[nodiscard]] ButtonBuilder hover_style(StyleOverlay overlay) &&;
  [[nodiscard]] ButtonBuilder focus_style(StyleOverlay overlay) &&;
  [[nodiscard]] ButtonBuilder disabled_style(StyleOverlay overlay) &&;
  [[nodiscard]] ButtonBuilder child(std::unique_ptr<Element> child) &&;
  [[nodiscard]] ButtonBuilder child(ElementBuilder child) &&;

  template <typename T>
    requires std::derived_from<T, Element> && (!std::same_as<T, Element>)
  [[nodiscard]] ButtonBuilder child(std::unique_ptr<T> child) && {
    return std::move(*this).child(std::unique_ptr<Element>(std::move(child)));
  }

  [[nodiscard]] ButtonBuilder on_click(ClickHandler handler) &&;
  [[nodiscard]] ButtonBuilder enabled(bool value) &&;
  [[nodiscard]] ButtonBuilder disabled() &&;
  [[nodiscard]] ButtonBuilder key(ElementKey key) &&;
  [[nodiscard]] ButtonBuilder key(std::string_view value) &&;
  [[nodiscard]] AnyElement build() &&;

 private:
  std::string action_name_;
  StyleState style_state_;
  ClickHandler click_handler_;
  std::unique_ptr<Element> child_;
  std::optional<ElementKey> key_;
  bool enabled_ = true;
};

[[nodiscard]] ButtonBuilder button(std::string_view action_name);

} // namespace cgpui

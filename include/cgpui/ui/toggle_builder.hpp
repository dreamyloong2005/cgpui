#pragma once

#include "cgpui/ui/element_builder_core.hpp"
#include "cgpui/ui/element_choice_nodes.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace cgpui {

class ToggleBuilder {
 public:
  ToggleBuilder(ToggleControlKind kind, std::string action_name);

  [[nodiscard]] ToggleBuilder style(Style style) &&;
  [[nodiscard]] ToggleBuilder hover_style(StyleOverlay overlay) &&;
  [[nodiscard]] ToggleBuilder focus_style(StyleOverlay overlay) &&;
  [[nodiscard]] ToggleBuilder active_style(StyleOverlay overlay) &&;
  [[nodiscard]] ToggleBuilder disabled_style(StyleOverlay overlay) &&;
  [[nodiscard]] ToggleBuilder label(std::string_view text) &&;
  [[nodiscard]] ToggleBuilder child(std::unique_ptr<Element> child) &&;
  [[nodiscard]] ToggleBuilder child(ElementBuilder child) &&;

  template <typename T>
    requires std::derived_from<T, Element> && (!std::same_as<T, Element>)
  [[nodiscard]] ToggleBuilder child(std::unique_ptr<T> child) && {
    return std::move(*this).child(std::unique_ptr<Element>(std::move(child)));
  }

  [[nodiscard]] ToggleBuilder checked(bool value) &&;
  [[nodiscard]] ToggleBuilder selected(bool value) &&;
  [[nodiscard]] ToggleBuilder on(bool value) &&;
  [[nodiscard]] ToggleBuilder on_click(ClickHandler handler) &&;
  [[nodiscard]] ToggleBuilder enabled(bool value) &&;
  [[nodiscard]] ToggleBuilder disabled() &&;
  [[nodiscard]] ToggleBuilder key(ElementKey key) &&;
  [[nodiscard]] ToggleBuilder key(std::string_view value) &&;
  [[nodiscard]] AnyElement build() &&;

 private:
  ToggleControlKind kind_;
  std::string action_name_;
  StyleState style_state_;
  ClickHandler click_handler_;
  std::unique_ptr<Element> child_;
  std::optional<ElementKey> key_;
  bool checked_ = false;
  bool enabled_ = true;
};

[[nodiscard]] ToggleBuilder checkbox(std::string_view action_name);
[[nodiscard]] ToggleBuilder radio(std::string_view action_name);
[[nodiscard]] ToggleBuilder toggle_switch(std::string_view action_name);

} // namespace cgpui

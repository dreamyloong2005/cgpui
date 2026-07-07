#pragma once

#include "cgpui/ui/element_builder_core.hpp"
#include "cgpui/ui/element_slider_nodes.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

class SliderBuilder {
 public:
  explicit SliderBuilder(std::string action_name);

  [[nodiscard]] SliderBuilder style(Style style) &&;
  [[nodiscard]] SliderBuilder hover_style(StyleOverlay overlay) &&;
  [[nodiscard]] SliderBuilder focus_style(StyleOverlay overlay) &&;
  [[nodiscard]] SliderBuilder active_style(StyleOverlay overlay) &&;
  [[nodiscard]] SliderBuilder disabled_style(StyleOverlay overlay) &&;
  [[nodiscard]] SliderBuilder range(float min, float max) &&;
  [[nodiscard]] SliderBuilder min(float value) &&;
  [[nodiscard]] SliderBuilder max(float value) &&;
  [[nodiscard]] SliderBuilder value(float value) &&;
  [[nodiscard]] SliderBuilder step(float value) &&;
  [[nodiscard]] SliderBuilder on_change(SliderChangeHandler handler) &&;
  [[nodiscard]] SliderBuilder enabled(bool value) &&;
  [[nodiscard]] SliderBuilder disabled() &&;
  [[nodiscard]] SliderBuilder key(ElementKey key) &&;
  [[nodiscard]] SliderBuilder key(std::string_view value) &&;
  [[nodiscard]] AnyElement build() &&;

 private:
  std::string action_name_;
  StyleState style_state_;
  SliderChangeHandler change_handler_;
  std::optional<ElementKey> key_;
  float min_ = 0.0F;
  float max_ = 1.0F;
  float value_ = 0.0F;
  float step_ = 0.0F;
  bool enabled_ = true;
};

[[nodiscard]] SliderBuilder slider(std::string_view action_name);

} // namespace cgpui

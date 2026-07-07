#pragma once

#include "cgpui/ui/element_core.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace cgpui {

using SliderChangeHandler =
    std::function<EventResult(float, const ElementEventContext&)>;

class SliderElement : public Element {
 public:
  SliderElement(
      std::string action_name,
      StyleState style_state,
      float min,
      float max,
      float value,
      float step,
      SliderChangeHandler change_handler = {});

  [[nodiscard]] std::string_view action_name() const;
  [[nodiscard]] const StyleState& style_state() const;
  [[nodiscard]] float min() const;
  [[nodiscard]] float max() const;
  [[nodiscard]] float value() const;
  [[nodiscard]] float step() const;
  [[nodiscard]] float normalized_value() const;
  void set_value(float value);
  [[nodiscard]] Rect track_rect() const;
  [[nodiscard]] Rect thumb_rect() const;
  [[nodiscard]] bool focusable() const override;
  [[nodiscard]] AccessibilityRole accessibility_role() const override;
  [[nodiscard]] std::string accessibility_name() const override;
  [[nodiscard]] std::string accessibility_value() const override;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  [[nodiscard]] ElementId hit_test(Point point) const override;
  void paint(PaintList& paint_list) const override;
  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override;
  [[nodiscard]] int z_index() const override;
  [[nodiscard]] int layer() const override;

 private:
  [[nodiscard]] float clamped_value(float value) const;
  [[nodiscard]] float value_for_point(Point point) const;

  std::string action_name_;
  StyleState style_state_;
  float min_ = 0.0F;
  float max_ = 1.0F;
  float value_ = 0.0F;
  float step_ = 0.0F;
  SliderChangeHandler change_handler_;
};

} // namespace cgpui

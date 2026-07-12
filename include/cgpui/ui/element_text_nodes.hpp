#pragma once

#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/text.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace cgpui {

class TextElement : public Element {
 public:
  explicit TextElement(TextModel* model, Style style = {})
      : model_(model),
        style_(std::move(style)),
        effective_style_(style_) {}

  [[nodiscard]] TextModel* model() const {
    return model_;
  }

  [[nodiscard]] std::string_view text() const {
    return model_ == nullptr ? std::string_view{} : model_->text();
  }

  [[nodiscard]] const Style& style() const {
    return style_;
  }

  [[nodiscard]] const Style& effective_style() const {
    return effective_style_;
  }

  [[nodiscard]] const FontDescriptor& font() const {
    return effective_style_.font;
  }

  [[nodiscard]] float font_size() const {
    return effective_style_.font_size;
  }

  [[nodiscard]] float glyph_width() const {
    const TextShapeRun run = shape_run();
    return run.glyphs.empty() ? font_size() * 0.5F : run.glyphs.front().advance;
  }

  [[nodiscard]] TextShapeRun shape_run() const {
    return shape_text(text(), font(), font_size());
  }

  [[nodiscard]] std::optional<Rect> caret_rect(DpiScale scale) const;
  [[nodiscard]] bool scroll_caret_into_view(
      ScrollState& state,
      DpiScale scale) const;

  [[nodiscard]] AccessibilityRole accessibility_role() const override {
    return AccessibilityRole::text;
  }

  [[nodiscard]] std::string accessibility_name() const override {
    return std::string(text());
  }

  [[nodiscard]] std::string accessibility_text() const override {
    return std::string(text());
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    const TextMeasurement measurement =
        measure_text(text(), font(), font_size(), input.scale);
    const TextWrapLayout wrap_layout =
        wrap_text_measurement(measurement, input.constraints.max_size.width);
    const Size preferred = wrap_layout.logical_size;
    const LayoutOutput output{
        .size = constrain_size(preferred, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

  void paint(PaintList& paint_list) const override;
  void inherit_text_style(const Style& style) override;

 private:
  TextModel* model_ = nullptr;
  Style style_;
  Style effective_style_;
};

class TextInputElement : public TextElement {
 public:
  explicit TextInputElement(TextModel& model, Style style = {})
      : TextElement(&model, std::move(style)) {}

  [[nodiscard]] bool focusable() const override {
    return true;
  }

  [[nodiscard]] AccessibilityRole accessibility_role() const override {
    return AccessibilityRole::text_input;
  }

  [[nodiscard]] std::string accessibility_value() const override {
    return std::string(text());
  }

  [[nodiscard]] AccessibilityPatternState accessibility_patterns()
      const override;

  [[nodiscard]] EventResult handle_accessibility_action(
      const AccessibilityActionRequested& action,
      const ElementEventContext& context) override;

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    (void)context;
    if (!enabled()) {
      return EventResult::unhandled();
    }
    if (std::holds_alternative<TextInput>(event) ||
        std::holds_alternative<ImeComposition>(event)) {
      return EventResult::consumed_event();
    }
    if (const auto* pointer = std::get_if<PointerButton>(&event);
        pointer != nullptr && pointer->pressed &&
        pointer->button == MouseButton::left) {
      return EventResult::consumed_event();
    }
    return EventResult::unhandled();
  }
};

class LabelElement : public Element {
 public:
  explicit LabelElement(std::string text, Style style = {})
      : text_(std::move(text)),
        style_(std::move(style)),
        effective_style_(style_) {}

  [[nodiscard]] std::string_view text() const {
    return text_;
  }

  [[nodiscard]] const Style& style() const {
    return style_;
  }

  [[nodiscard]] const Style& effective_style() const {
    return effective_style_;
  }

  [[nodiscard]] const FontDescriptor& font() const {
    return effective_style_.font;
  }

  [[nodiscard]] float font_size() const {
    return effective_style_.font_size;
  }

  [[nodiscard]] float glyph_width() const {
    const TextShapeRun run = shape_run();
    return run.glyphs.empty() ? font_size() * 0.5F : run.glyphs.front().advance;
  }

  [[nodiscard]] TextShapeRun shape_run() const {
    return shape_text(text_, font(), font_size());
  }

  [[nodiscard]] AccessibilityRole accessibility_role() const override {
    return AccessibilityRole::label;
  }

  [[nodiscard]] std::string accessibility_name() const override {
    return std::string(text_);
  }

  [[nodiscard]] std::string accessibility_text() const override {
    return std::string(text_);
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    const TextMeasurement measurement =
        measure_text(text_, font(), font_size(), input.scale);
    const TextWrapLayout wrap_layout =
        wrap_text_measurement(measurement, input.constraints.max_size.width);
    const Size preferred = wrap_layout.logical_size;
    const LayoutOutput output{
        .size = constrain_size(preferred, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

  void paint(PaintList& paint_list) const override;
  void inherit_text_style(const Style& style) override;

  [[nodiscard]] int z_index() const override {
    return style_.z_index;
  }

  [[nodiscard]] int layer() const override {
    return style_.layer;
  }

 private:
  std::string text_;
  Style style_;
  Style effective_style_;
};

} // namespace cgpui

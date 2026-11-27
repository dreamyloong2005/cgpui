#pragma once

#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/text.hpp"

#include <string>
#include <string_view>

namespace cgpui {

class TextElement : public Element {
 public:
  explicit TextElement(TextModel* model, Style style = {});

  [[nodiscard]] TextModel* model() const;

  [[nodiscard]] std::string_view text() const;

  [[nodiscard]] const Style& style() const;

  [[nodiscard]] const Style& effective_style() const;

  [[nodiscard]] const FontDescriptor& font() const;

  [[nodiscard]] float font_size() const;

  [[nodiscard]] float glyph_width() const;

  [[nodiscard]] TextShapeRun shape_run() const;

  // While composing, caret_rect is the composition caret and
  // composition_start_rect the caret before the composition text.
  // The model text, with the IME composition in place of the selection
  // while composing.
  [[nodiscard]] std::string display_text() const;
  [[nodiscard]] std::optional<Rect> caret_rect(DpiScale scale) const;
  [[nodiscard]] std::optional<Rect> composition_start_rect(
      DpiScale scale) const;
  [[nodiscard]] bool scroll_caret_into_view(
      ScrollState& state,
      DpiScale scale) const;

  [[nodiscard]] AccessibilityRole accessibility_role() const override;

  [[nodiscard]] std::string accessibility_name() const override;

  [[nodiscard]] std::string accessibility_text() const override;

  // Measures the composition too, so it does not wrap while composing.
  // The style's size, padding and margin give a CSS content box around
  // the text.
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  // The content box the text is drawn, hit-tested and caret-placed in.
  [[nodiscard]] std::optional<Rect> text_bounds() const;

  void paint(PaintList& paint_list) const override;
  void inherit_text_style(const Style& style) override;

 private:
  TextModel* model_ = nullptr;
  Style style_;
  Style effective_style_;
};

class TextInputElement : public TextElement {
 public:
  explicit TextInputElement(TextModel& model, Style style = {});

  [[nodiscard]] bool focusable() const override;

  [[nodiscard]] AccessibilityRole accessibility_role() const override;

  [[nodiscard]] std::string accessibility_value() const override;

  [[nodiscard]] AccessibilityPatternState accessibility_patterns()
      const override;

  [[nodiscard]] EventResult handle_accessibility_action(
      const AccessibilityActionRequested& action,
      const ElementEventContext& context) override;

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override;
};

class LabelElement : public Element {
 public:
  explicit LabelElement(std::string text, Style style = {});

  [[nodiscard]] std::string_view text() const;

  [[nodiscard]] const Style& style() const;

  [[nodiscard]] const Style& effective_style() const;

  [[nodiscard]] const FontDescriptor& font() const;

  [[nodiscard]] float font_size() const;

  [[nodiscard]] float glyph_width() const;

  [[nodiscard]] TextShapeRun shape_run() const;

  [[nodiscard]] AccessibilityRole accessibility_role() const override;

  [[nodiscard]] std::string accessibility_name() const override;

  [[nodiscard]] std::string accessibility_text() const override;

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  [[nodiscard]] std::optional<Rect> text_bounds() const;

  void paint(PaintList& paint_list) const override;
  void inherit_text_style(const Style& style) override;

  [[nodiscard]] int z_index() const override;

  [[nodiscard]] int layer() const override;

 private:
  std::string text_;
  Style style_;
  Style effective_style_;
};

} // namespace cgpui

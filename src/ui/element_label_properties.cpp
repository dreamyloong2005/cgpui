#include "cgpui/ui/element_text_nodes.hpp"

#include <utility>

namespace cgpui {

std::string_view LabelElement::text() const {
  return text_;
}

const Style& LabelElement::style() const {
  return style_;
}

const Style& LabelElement::effective_style() const {
  return effective_style_;
}

const FontDescriptor& LabelElement::font() const {
  return effective_style_.font;
}

float LabelElement::font_size() const {
  return effective_style_.font_size;
}

float LabelElement::glyph_width() const {
  const TextShapeRun run = shape_run();
  return run.glyphs.empty() ? font_size() * 0.5F : run.glyphs.front().advance;
}

TextShapeRun LabelElement::shape_run() const {
  return shape_text(text_, font(), font_size());
}

AccessibilityRole LabelElement::accessibility_role() const {
  return AccessibilityRole::label;
}

std::string LabelElement::accessibility_name() const {
  return std::string(text_);
}

std::string LabelElement::accessibility_text() const {
  return std::string(text_);
}

int LabelElement::z_index() const {
  return style_.z_index;
}

int LabelElement::layer() const {
  return style_.layer;
}

LabelElement::LabelElement(std::string text, Style style)
    : text_(std::move(text)),
      style_(std::move(style)),
      effective_style_(style_) {}

} // namespace cgpui

#include "cgpui/ui/element_text_nodes.hpp"

#include <utility>

namespace cgpui {

TextModel* TextElement::model() const {
  return model_;
}

std::string_view TextElement::text() const {
  return model_ == nullptr ? std::string_view{} : model_->text();
}

const Style& TextElement::style() const {
  return style_;
}

const Style& TextElement::effective_style() const {
  return effective_style_;
}

const FontDescriptor& TextElement::font() const {
  return effective_style_.font;
}

float TextElement::font_size() const {
  return effective_style_.font_size;
}

float TextElement::glyph_width() const {
  const TextShapeRun run = shape_run();
  return run.glyphs.empty() ? font_size() * 0.5F : run.glyphs.front().advance;
}

TextShapeRun TextElement::shape_run() const {
  return shape_text(text(), font(), font_size());
}

AccessibilityRole TextElement::accessibility_role() const {
  return AccessibilityRole::text;
}

std::string TextElement::accessibility_name() const {
  return std::string(text());
}

std::string TextElement::accessibility_text() const {
  return std::string(text());
}

TextElement::TextElement(TextModel* model, Style style)
    : model_(model),
      style_(std::move(style)),
      effective_style_(style_) {}

} // namespace cgpui

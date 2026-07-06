#include "text_style_inheritance.hpp"

namespace cgpui {

bool text_style_has_font(const Style& style) {
  return style.authored_text.font || style.font != FontDescriptor{};
}

bool text_style_has_font_size(const Style& style) {
  return style.authored_text.font_size ||
         style.font_size != Style{}.font_size;
}

Style inherited_text_style(const Style& style) {
  Style inherited;
  inherited.foreground_color = style.foreground_color;
  if (text_style_has_font(style)) {
    inherited.font = style.font;
    inherited.authored_text.font = true;
  }
  if (text_style_has_font_size(style)) {
    inherited.font_size = style.font_size;
    inherited.authored_text.font_size = true;
  }
  return inherited;
}

Style merge_inherited_text_style(const Style& inherited, const Style& local) {
  Style merged = local;
  if (!merged.foreground_color.has_value() &&
      inherited.foreground_color.has_value()) {
    merged.foreground_color = inherited.foreground_color;
  }
  if (!text_style_has_font(merged) && text_style_has_font(inherited)) {
    merged.font = inherited.font;
    merged.authored_text.font = true;
  }
  if (!text_style_has_font_size(merged) &&
      text_style_has_font_size(inherited)) {
    merged.font_size = inherited.font_size;
    merged.authored_text.font_size = true;
  }
  return merged;
}

} // namespace cgpui

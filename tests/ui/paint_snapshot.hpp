#pragma once

#include "cgpui/ui/ui.hpp"

#include <iomanip>
#include <sstream>
#include <span>
#include <string>
#include <vector>

namespace {

[[nodiscard]] std::string snapshot_float(float value) {
  std::ostringstream out;
  out << std::fixed << std::setprecision(1) << value;
  return out.str();
}

[[nodiscard]] std::string snapshot_color_float(float value) {
  std::ostringstream out;
  out << std::fixed << std::setprecision(3) << value;
  return out.str();
}

[[nodiscard]] std::string snapshot_rect(cgpui::Rect rect) {
  std::ostringstream out;
  out << "(" << snapshot_float(rect.origin.x) << ","
      << snapshot_float(rect.origin.y) << " "
      << snapshot_float(rect.size.width) << "x"
      << snapshot_float(rect.size.height) << ")";
  return out.str();
}

[[nodiscard]] std::string snapshot_color(cgpui::Color color) {
  std::ostringstream out;
  out << snapshot_color_float(color.r) << "," << snapshot_color_float(color.g)
      << "," << snapshot_color_float(color.b) << ","
      << snapshot_color_float(color.a);
  return out.str();
}

[[nodiscard]] std::string snapshot_transform(
    cgpui::AffineTransform transform) {
  std::ostringstream out;
  out << "[" << snapshot_float(transform.scale_x) << ","
      << snapshot_float(transform.skew_y) << ","
      << snapshot_float(transform.skew_x) << ","
      << snapshot_float(transform.scale_y) << ","
      << snapshot_float(transform.translate_x) << ","
      << snapshot_float(transform.translate_y) << "]";
  return out.str();
}

[[nodiscard]] std::string snapshot_clip(const std::optional<cgpui::Rect>& clip) {
  return clip.has_value() ? snapshot_rect(*clip) : "none";
}

void append_snapshot_metadata(
    std::ostringstream& out,
    const std::optional<cgpui::Rect>& clip,
    cgpui::PaintMetadata metadata) {
  out << " clip=" << snapshot_clip(clip)
      << " opacity=" << snapshot_color_float(metadata.opacity)
      << " transform=" << snapshot_transform(metadata.transform);
}

void append_snapshot_rect(
    std::ostringstream& out,
    cgpui::Rect rect,
    cgpui::Color color) {
  out << " rect=" << snapshot_rect(rect)
      << " color=" << snapshot_color(color);
}

void append_snapshot_shadow(
    std::ostringstream& out,
    const cgpui::BoxShadowPaint& shadow) {
  out << " bounds=" << snapshot_rect(shadow.bounds)
      << " color=" << snapshot_color(shadow.shadow.color)
      << " offset=" << snapshot_float(shadow.shadow.offset.x) << ","
      << snapshot_float(shadow.shadow.offset.y)
      << " blur=" << snapshot_float(shadow.shadow.blur_radius)
      << " spread=" << snapshot_float(shadow.shadow.spread_radius)
      << " radius=" << snapshot_float(shadow.radius.top_left) << ","
      << snapshot_float(shadow.radius.top_right) << ","
      << snapshot_float(shadow.radius.bottom_right) << ","
      << snapshot_float(shadow.radius.bottom_left);
}

void append_snapshot_text(
    std::ostringstream& out,
    cgpui::Rect bounds,
    cgpui::Color color,
    const cgpui::FontDescriptor& font,
    std::string_view content,
    float font_size,
    float device_font_size,
    std::size_t glyph_count) {
  out << " bounds=" << snapshot_rect(bounds)
      << " color=" << snapshot_color(color)
      << " content=\"" << content << "\" font="
      << (font.family.empty() ? "<default>" : font.family)
      << " size=" << snapshot_float(font_size)
      << " device_size=" << snapshot_float(device_font_size)
      << " glyphs=" << glyph_count;
}

[[nodiscard]] std::string snapshot_paint_commands(
    std::span<const cgpui::PaintCommand> commands) {
  std::ostringstream out;
  for (std::size_t index = 0; index < commands.size(); ++index) {
    const cgpui::PaintCommand& command = commands[index];
    out << index << " ";
    switch (command.kind) {
      case cgpui::PaintCommandKind::solid_rect:
        out << "solid_rect";
        append_snapshot_rect(
            out,
            command.solid_rect.rect,
            command.solid_rect.color);
        break;
      case cgpui::PaintCommandKind::rounded_rect:
        out << "rounded_rect";
        append_snapshot_rect(
            out,
            command.rounded_rect.rect,
            command.rounded_rect.color);
        out << " radius="
            << snapshot_float(command.rounded_rect.radius.top_left) << ","
            << snapshot_float(command.rounded_rect.radius.top_right) << ","
            << snapshot_float(command.rounded_rect.radius.bottom_right) << ","
            << snapshot_float(command.rounded_rect.radius.bottom_left);
        break;
      case cgpui::PaintCommandKind::box_shadow:
        out << "box_shadow";
        append_snapshot_shadow(out, command.box_shadow);
        break;
      case cgpui::PaintCommandKind::text:
        out << "text";
        append_snapshot_text(
            out,
            command.text.bounds,
            command.text.color,
            command.text.font,
            command.text.content,
            command.text.font_size,
            command.text.device_font_size,
            command.text.glyphs.size());
        break;
      case cgpui::PaintCommandKind::text_selection:
        out << "text_selection rect="
            << snapshot_rect(command.text_selection.rect)
            << " color=" << snapshot_color(command.text_selection.color)
            << " range=" << command.text_selection.range.start << ".."
            << command.text_selection.range.end
            << " size=" << snapshot_float(command.text_selection.font_size);
        break;
      case cgpui::PaintCommandKind::text_caret:
        out << "text_caret rect=" << snapshot_rect(command.text_caret.rect)
            << " color=" << snapshot_color(command.text_caret.color)
            << " byte=" << command.text_caret.byte_offset
            << " size=" << snapshot_float(command.text_caret.font_size);
        break;
    }
    append_snapshot_metadata(out, command.clip_rect, command.metadata);
    out << "\n";
  }
  return out.str();
}

[[nodiscard]] std::string snapshot_render_commands(
    std::span<const cgpui::SolidRect> rects,
    std::span<const cgpui::RoundedRectDraw> rounded_rects,
    std::span<const cgpui::TextDraw> texts) {
  std::ostringstream out;
  std::size_t index = 0;
  for (const cgpui::SolidRect& rect : rects) {
    out << index++ << " solid_rect";
    append_snapshot_rect(out, rect.rect, rect.color);
    append_snapshot_metadata(out, rect.clip_rect, rect.metadata);
    out << "\n";
  }
  for (const cgpui::RoundedRectDraw& rect : rounded_rects) {
    out << index++ << " rounded_rect";
    append_snapshot_rect(out, rect.rect, rect.color);
    out << " radius=" << snapshot_float(rect.radius.top_left) << ","
        << snapshot_float(rect.radius.top_right) << ","
        << snapshot_float(rect.radius.bottom_right) << ","
        << snapshot_float(rect.radius.bottom_left);
    append_snapshot_metadata(out, rect.clip_rect, rect.metadata);
    out << "\n";
  }
  for (const cgpui::TextDraw& text : texts) {
    out << index++ << " text";
    append_snapshot_text(
        out,
        text.bounds,
        text.color,
        text.font,
        text.content,
        text.font_size,
        text.device_font_size,
        text.glyphs.size());
    append_snapshot_metadata(out, text.clip_rect, text.metadata);
    out << "\n";
  }
  return out.str();
}

[[nodiscard]] std::string snapshot_render_commands(
    std::span<const cgpui::SolidRect> rects,
    std::span<const cgpui::TextDraw> texts) {
  return snapshot_render_commands(
      rects,
      std::span<const cgpui::RoundedRectDraw>{},
      texts);
}

} // namespace

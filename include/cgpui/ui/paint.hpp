#pragma once

#include "cgpui/core/entity.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/clipboard.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text.hpp"

#include <any>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace cgpui {

enum class PaintCommandKind {
  solid_rect,
  rounded_rect,
  box_shadow,
  text,
  text_selection,
  text_caret,
  image,
};

struct RoundedRect {
  Rect rect;
  Color color;
  BorderRadii radius;
};

struct BoxShadowPaint {
  Rect bounds;
  BoxShadow shadow;
  BorderRadii radius;
};

struct TextPaint {
  Rect bounds;
  Color color;
  FontDescriptor font;
  std::string content;
  std::size_t byte_length = 0;
  float font_size = 16.0F;
  DpiScale scale;
  float device_font_size = 16.0F;
  std::vector<TextGlyphPaint> glyphs;
  std::vector<TextWrapLine> lines;
};

struct TextSelectionPaint {
  Rect rect;
  Color color;
  TextSelectionRange range;
  float font_size = 16.0F;
};

struct TextCaretPaint {
  Rect rect;
  Color color;
  std::size_t byte_offset = 0;
  float font_size = 16.0F;
};

struct ImagePaint {
  Rect bounds;
  ImageAssetDescriptor asset;
  std::optional<Rect> source_rect;
  std::optional<Color> tint;
};

struct ImeCandidateRect {
  ElementId element_id;
  Rect rect;
  std::size_t byte_offset = 0;
};

struct PaintCommand {
  PaintCommandKind kind = PaintCommandKind::solid_rect;
  SolidRect solid_rect;
  RoundedRect rounded_rect;
  BoxShadowPaint box_shadow;
  TextPaint text;
  TextSelectionPaint text_selection;
  TextCaretPaint text_caret;
  ImagePaint image;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

class PaintList {
 public:
  void clear();
  void set_scale(DpiScale scale);
  [[nodiscard]] DpiScale scale() const;
  void push_clip(Rect rect);
  void pop_clip();
  void push_metadata(PaintMetadata metadata);
  void pop_metadata();
  void fill_rect(Rect rect, Color color);
  void fill_rounded_rect(Rect rect, Color color, BorderRadii radius);
  void draw_box_shadow(Rect bounds, BoxShadow shadow, BorderRadii radius = {});
  void fill_text(
      Rect bounds,
      Color color,
      std::string_view text,
      FontDescriptor font = {},
      float font_size = 16.0F);
  void fill_text_selection(
      Rect rect,
      Color color,
      TextSelectionRange range,
      float font_size = 16.0F);
  void fill_text_caret(
      Rect rect,
      Color color,
      std::size_t byte_offset,
      float font_size = 16.0F);
  void draw_image(
      Rect bounds,
      ImageAssetDescriptor asset,
      std::optional<Rect> source_rect = std::nullopt,
      std::optional<Color> tint = std::nullopt);
  void set_text_measurement_cache(TextMeasurementCache* cache);
  [[nodiscard]] std::span<const PaintCommand> commands() const;

 private:
  std::vector<PaintCommand> commands_;
  std::vector<Rect> clip_stack_;
  std::vector<PaintMetadata> metadata_stack_;
  DpiScale scale_;
  TextMeasurementCache* text_measurement_cache_ = nullptr;
};


} // namespace cgpui

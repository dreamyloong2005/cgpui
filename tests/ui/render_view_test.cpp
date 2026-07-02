#include "cgpui/ui/ui.hpp"
#include "paint_snapshot.hpp"

#include <memory>
#include <string>
#include <vector>

namespace {

bool same_transform(cgpui::AffineTransform lhs, cgpui::AffineTransform rhs) {
  return lhs.scale_x == rhs.scale_x && lhs.skew_y == rhs.skew_y &&
         lhs.skew_x == rhs.skew_x && lhs.scale_y == rhs.scale_y &&
         lhs.translate_x == rhs.translate_x &&
         lhs.translate_y == rhs.translate_y;
}

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override { clear_count += 1; }

  void draw_rect(const cgpui::SolidRect& rect) override {
    draw_count += 1;
    last_rect = rect;
    rects.push_back(rect);
  }

  void draw_rounded_rect(const cgpui::RoundedRectDraw& rect) override {
    rounded_draw_count += 1;
    last_rounded_rect = rect;
    rounded_rects.push_back(rect);
  }

  void draw_text(const cgpui::TextDraw& text) override {
    text_draw_count += 1;
    last_text = text;
    texts.push_back(text);
  }

  void draw_text_selection(const cgpui::TextSelectionDraw& selection) override {
    text_selection_draw_count += 1;
    last_text_selection = selection;
    text_selections.push_back(selection);
  }

  void draw_text_caret(const cgpui::TextCaretDraw& caret) override {
    text_caret_draw_count += 1;
    last_text_caret = caret;
    text_carets.push_back(caret);
  }

  cgpui::Result<void> present() override {
    present_count += 1;
    return {};
  }

  int clear_count = 0;
  int draw_count = 0;
  int rounded_draw_count = 0;
  int text_draw_count = 0;
  int text_selection_draw_count = 0;
  int text_caret_draw_count = 0;
  int present_count = 0;
  cgpui::SolidRect last_rect;
  cgpui::RoundedRectDraw last_rounded_rect;
  cgpui::TextDraw last_text;
  cgpui::TextSelectionDraw last_text_selection;
  cgpui::TextCaretDraw last_text_caret;
  std::vector<cgpui::SolidRect> rects;
  std::vector<cgpui::RoundedRectDraw> rounded_rects;
  std::vector<cgpui::TextDraw> texts;
  std::vector<cgpui::TextSelectionDraw> text_selections;
  std::vector<cgpui::TextCaretDraw> text_carets;
};

class RecordingRenderer final : public cgpui::Renderer {
 public:
  explicit RecordingRenderer(RecordingFrame& frame) : frame_(frame) {}

  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unique_ptr<cgpui::RenderFrame>(new BorrowedFrame(frame_));
  }

 private:
  class BorrowedFrame final : public cgpui::RenderFrame {
   public:
    explicit BorrowedFrame(RecordingFrame& frame) : frame_(frame) {}

    void clear(cgpui::Color color) override { frame_.clear(color); }
    void draw_rect(const cgpui::SolidRect& rect) override {
      frame_.draw_rect(rect);
    }
    void draw_rounded_rect(const cgpui::RoundedRectDraw& rect) override {
      frame_.draw_rounded_rect(rect);
    }
    void draw_text(const cgpui::TextDraw& text) override {
      frame_.draw_text(text);
    }
    void draw_text_selection(const cgpui::TextSelectionDraw& selection) override {
      frame_.draw_text_selection(selection);
    }
    void draw_text_caret(const cgpui::TextCaretDraw& caret) override {
      frame_.draw_text_caret(caret);
    }
    cgpui::Result<void> present() override { return frame_.present(); }

   private:
    RecordingFrame& frame_;
  };

  RecordingFrame& frame_;
};

class EmptyFrameRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unique_ptr<cgpui::RenderFrame>{};
  }
};

class EmptyView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class TextOnlyView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.push_metadata(cgpui::PaintMetadata{
        .opacity = 0.5F,
        .transform = cgpui::AffineTransform::translation(3.0F, 4.0F),
    });
    paint_list.fill_text(
        cgpui::Rect{.origin = {2.0F, 4.0F},
                    .size = {.width = 24.0F, .height = 16.0F}},
        cgpui::Color{.r = 0.8F, .g = 0.9F, .b = 1.0F, .a = 1.0F},
        "abc");
    paint_list.pop_metadata();
  }
};

class RoundedBoxView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.push_clip(cgpui::Rect{
        .origin = {.x = 1.0F, .y = 2.0F},
        .size = {.width = 48.0F, .height = 32.0F},
    });
    paint_list.push_metadata(cgpui::PaintMetadata{
        .opacity = 0.75F,
        .transform = cgpui::AffineTransform::translation(6.0F, 8.0F),
    });
    paint_list.fill_rounded_rect(
        cgpui::Rect{
            .origin = {.x = 4.0F, .y = 5.0F},
            .size = {.width = 30.0F, .height = 16.0F},
        },
        cgpui::Color{.r = 0.4F, .g = 0.5F, .b = 0.6F, .a = 1.0F},
        cgpui::BorderRadii::corners(2.0F, 3.0F, 4.0F, 5.0F));
    paint_list.pop_metadata();
    paint_list.pop_clip();
  }
};

class TextSelectionCaretView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.push_clip(cgpui::Rect{
        .origin = {.x = 1.0F, .y = 2.0F},
        .size = {.width = 96.0F, .height = 28.0F},
    });
    paint_list.push_metadata(cgpui::PaintMetadata{
        .opacity = 0.625F,
        .transform = cgpui::AffineTransform::translation(4.0F, 6.0F),
    });
    paint_list.fill_text_selection(
        cgpui::Rect{
            .origin = {.x = 12.0F, .y = 14.0F},
            .size = {.width = 30.0F, .height = 18.0F},
        },
        cgpui::Color{.r = 0.2F, .g = 0.35F, .b = 0.9F, .a = 0.5F},
        cgpui::TextSelectionRange{.start = 2, .end = 6, .collapsed = false},
        18.0F);
    paint_list.fill_text_caret(
        cgpui::Rect{
            .origin = {.x = 42.0F, .y = 14.0F},
            .size = {.width = 1.0F, .height = 18.0F},
        },
        cgpui::Color{.r = 0.95F, .g = 0.95F, .b = 1.0F, .a = 1.0F},
        6,
        18.0F);
    paint_list.pop_metadata();
    paint_list.pop_clip();
  }
};

} // namespace

int main() {
  {
    EmptyFrameRenderer renderer;
    EmptyView view;

    const auto result =
        cgpui::render_view(renderer, view, cgpui::Size{10.0F, 10.0F});
    if (result) {
      return 1;
    }

    if (result.error().code != cgpui::ErrorCode::frame_acquisition_failed) {
      return 2;
    }
  }

  RecordingFrame frame;
  RecordingRenderer renderer(frame);
  TextOnlyView view;
  const auto result =
      cgpui::render_view(renderer, view, cgpui::Size{64.0F, 64.0F});
  if (!result) {
    return 3;
  }
  if (frame.clear_count != 1 || frame.present_count != 1) {
    return 4;
  }

  if (frame.draw_count != 0 || frame.text_draw_count != 1) {
    return 5;
  }
  if (frame.last_text.content != "abc" || frame.last_text.glyphs.size() != 3) {
    return 6;
  }
  if (frame.last_text.metadata.opacity != 0.5F ||
      !same_transform(
          frame.last_text.metadata.transform,
          cgpui::AffineTransform::translation(3.0F, 4.0F))) {
    return 8;
  }

  if (frame.last_text.glyphs[2].key.byte_offset != 2 ||
      frame.last_text.glyphs[2].origin.x != 18.0F) {
    return 7;
  }

  const cgpui::TexturedGlyphQuad glyph_quad{
      .key = frame.last_text.glyphs[2].key,
      .device_bounds =
          cgpui::Rect{
              .origin = frame.last_text.glyphs[2].device_origin,
              .size = {.width = 8.0F, .height = 16.0F},
          },
      .atlas_bounds = cgpui::Rect{.size = {.width = 8.0F, .height = 16.0F}},
      .atlas_uv_bounds =
          cgpui::Rect{.size = {.width = 8.0F / 256.0F,
                               .height = 16.0F / 256.0F}},
      .color = frame.last_text.color,
      .clip_rect = frame.last_text.clip_rect,
      .metadata = frame.last_text.metadata,
  };
  if (glyph_quad.device_bounds.origin.x != 18.0F ||
      glyph_quad.metadata.opacity != 0.5F ||
      !same_transform(
          glyph_quad.metadata.transform,
          cgpui::AffineTransform::translation(3.0F, 4.0F))) {
    return 9;
  }

  const std::string snapshot =
      snapshot_render_commands(frame.rects, frame.texts);
  const std::string expected =
      "0 text bounds=(2.0,4.0 24.0x16.0) color=0.800,0.900,1.000,1.000 content=\"abc\" font=<default> size=16.0 device_size=16.0 glyphs=3 clip=none opacity=0.500 transform=[1.0,0.0,0.0,1.0,3.0,4.0]\n";
  if (snapshot != expected) {
    return 10;
  }

  RecordingFrame rounded_frame;
  RecordingRenderer rounded_renderer(rounded_frame);
  RoundedBoxView rounded_view;
  const auto rounded_result = cgpui::render_view(
      rounded_renderer,
      rounded_view,
      cgpui::Size{64.0F, 64.0F});
  if (!rounded_result) {
    return 11;
  }
  if (rounded_frame.draw_count != 0 ||
      rounded_frame.rounded_draw_count != 1 ||
      rounded_frame.text_draw_count != 0) {
    return 12;
  }
  const cgpui::RoundedRectDraw& rounded = rounded_frame.last_rounded_rect;
  if (rounded.rect.origin.x != 4.0F || rounded.rect.origin.y != 5.0F ||
      rounded.rect.size.width != 30.0F || rounded.radius.top_right != 3.0F ||
      !rounded.clip_rect.has_value() ||
      rounded.clip_rect->size.width != 48.0F ||
      rounded.metadata.opacity != 0.75F ||
      !same_transform(
          rounded.metadata.transform,
          cgpui::AffineTransform::translation(6.0F, 8.0F))) {
    return 13;
  }

  const std::string rounded_snapshot = snapshot_render_commands(
      rounded_frame.rects,
      rounded_frame.rounded_rects,
      rounded_frame.texts);
  const std::string rounded_expected =
      "0 rounded_rect rect=(4.0,5.0 30.0x16.0) color=0.400,0.500,0.600,1.000 radius=2.0,3.0,4.0,5.0 clip=(1.0,2.0 48.0x32.0) opacity=0.750 transform=[1.0,0.0,0.0,1.0,6.0,8.0]\n";
  if (rounded_snapshot != rounded_expected) {
    return 14;
  }

  RecordingFrame selection_frame;
  RecordingRenderer selection_renderer(selection_frame);
  TextSelectionCaretView selection_view;
  cgpui::FrameStatistics selection_stats;
  const auto selection_result = cgpui::render_view(
      selection_renderer,
      selection_view,
      cgpui::Size{128.0F, 48.0F},
      cgpui::DpiScale{},
      &selection_stats);
  if (!selection_result) {
    return 15;
  }
  if (selection_frame.text_selection_draw_count != 1 ||
      selection_frame.text_caret_draw_count != 1 ||
      selection_frame.draw_count != 0 ||
      selection_frame.text_draw_count != 0 ||
      selection_stats.skipped_command_count != 0 ||
      selection_stats.submitted_command_count != 2 ||
      selection_stats.text_selection_command_count != 1 ||
      selection_stats.text_caret_command_count != 1) {
    return 16;
  }
  const cgpui::TextSelectionDraw& selection =
      selection_frame.last_text_selection;
  const cgpui::TextCaretDraw& caret = selection_frame.last_text_caret;
  if (selection.rect.origin.x != 12.0F ||
      selection.range.start != 2 ||
      selection.range.end != 6 ||
      selection.range.collapsed ||
      selection.color.b != 0.9F ||
      !selection.clip_rect.has_value() ||
      selection.clip_rect->size.width != 96.0F ||
      selection.metadata.opacity != 0.625F ||
      !same_transform(
          selection.metadata.transform,
          cgpui::AffineTransform::translation(4.0F, 6.0F))) {
    return 17;
  }

  return caret.rect.origin.x == 42.0F && caret.byte_offset == 6 &&
                 caret.color.a == 1.0F && caret.clip_rect.has_value() &&
                 caret.metadata.opacity == 0.625F
             ? 0
             : 18;
}

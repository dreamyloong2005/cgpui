#include "cgpui/ui/ui.hpp"
#include "paint_snapshot.hpp"

#include <functional>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
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

  void draw_image(const cgpui::ImageDraw& image) override {
    image_draw_count += 1;
    last_image = image;
    images.push_back(image);
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
  int image_draw_count = 0;
  int present_count = 0;
  cgpui::SolidRect last_rect;
  cgpui::RoundedRectDraw last_rounded_rect;
  cgpui::TextDraw last_text;
  cgpui::TextSelectionDraw last_text_selection;
  cgpui::TextCaretDraw last_text_caret;
  cgpui::ImageDraw last_image;
  std::vector<cgpui::SolidRect> rects;
  std::vector<cgpui::RoundedRectDraw> rounded_rects;
  std::vector<cgpui::TextDraw> texts;
  std::vector<cgpui::TextSelectionDraw> text_selections;
  std::vector<cgpui::TextCaretDraw> text_carets;
  std::vector<cgpui::ImageDraw> images;
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
    void draw_image(const cgpui::ImageDraw& image) override {
      frame_.draw_image(image);
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

class ThemeFakeWindow final : public cgpui::PlatformWindow {
 public:
  explicit ThemeFakeWindow(cgpui::WindowState state) : state_(state) {}

  [[nodiscard]] cgpui::NativeSurfaceHandle native_surface() const override {
    return cgpui::Win32SurfaceHandle{};
  }

  [[nodiscard]] cgpui::WindowState state() const override { return state_; }

  void request_redraw() override { request_redraw_count += 1; }
  void request_close() override { request_close_count += 1; }
  void set_title(std::string_view title) override { last_title = title; }
  void set_cursor(cgpui::CursorShape cursor_shape) override {
    last_cursor_shape = cursor_shape;
  }
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement> placement) override {
    last_ime_placement = placement;
  }
  void update_accessibility_tree(
      cgpui::PlatformAccessibilityTreeUpdate update) override {
    last_accessibility_update = std::move(update);
  }
  void emit(const cgpui::PlatformEvent& event) {
    if (callback) {
      callback(event);
    }
  }

  cgpui::PlatformEventCallback callback;
  int request_redraw_count = 0;
  int request_close_count = 0;
  std::string last_title;
  cgpui::CursorShape last_cursor_shape = cgpui::CursorShape::default_arrow;
  std::optional<cgpui::ImeTextInputPlacement> last_ime_placement;
  std::optional<cgpui::PlatformAccessibilityTreeUpdate>
      last_accessibility_update;

 private:
  cgpui::WindowState state_;
};

class ThemeFakeApplication final : public cgpui::PlatformApplication {
 public:
  explicit ThemeFakeApplication(ThemeFakeWindow& window) : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    create_window_count += 1;
    last_descriptor = descriptor;
    window_.callback = std::move(callback);
    return std::unique_ptr<cgpui::PlatformWindow>(
        new BorrowedWindow(window_));
  }

  int run() override {
    run_count += 1;
    if (on_run) {
      on_run();
    }
    return run_result;
  }

  void quit() override { quit_count += 1; }

  ThemeFakeWindow& window_;
  int create_window_count = 0;
  int run_count = 0;
  int quit_count = 0;
  int run_result = 0;
  cgpui::WindowDescriptor last_descriptor{};
  std::function<void()> on_run;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(ThemeFakeWindow& window) : window_(window) {}

    [[nodiscard]] cgpui::NativeSurfaceHandle native_surface() const override {
      return window_.native_surface();
    }
    [[nodiscard]] cgpui::WindowState state() const override {
      return window_.state();
    }
    void request_redraw() override { window_.request_redraw(); }
    void request_close() override { window_.request_close(); }
    void set_title(std::string_view title) override {
      window_.set_title(title);
    }
    void set_cursor(cgpui::CursorShape cursor_shape) override {
      window_.set_cursor(cursor_shape);
    }
    void set_ime_text_input_placement(
        std::optional<cgpui::ImeTextInputPlacement> placement) override {
      window_.set_ime_text_input_placement(placement);
    }
    void update_accessibility_tree(
        cgpui::PlatformAccessibilityTreeUpdate update) override {
      window_.update_accessibility_tree(std::move(update));
    }

   private:
    ThemeFakeWindow& window_;
  };
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

class WrappedTextView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.fill_text(
        cgpui::Rect{.origin = {2.0F, 4.0F},
                    .size = {.width = 16.0F, .height = 16.0F}},
        cgpui::Color{.r = 0.9F, .g = 0.8F, .b = 0.7F, .a = 1.0F},
        "abcde",
        cgpui::FontDescriptor{},
        16.0F);
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

class NestedClipStackView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.push_clip(cgpui::Rect{
        .origin = {.x = 0.0F, .y = 0.0F},
        .size = {.width = 128.0F, .height = 64.0F},
    });
    paint_list.push_clip(cgpui::Rect{
        .origin = {.x = 8.0F, .y = 10.0F},
        .size = {.width = 48.0F, .height = 24.0F},
    });
    paint_list.fill_rect(
        cgpui::Rect{
            .origin = {.x = 12.0F, .y = 14.0F},
            .size = {.width = 16.0F, .height = 12.0F},
        },
        cgpui::Color{.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 1.0F});
    paint_list.pop_clip();
    paint_list.pop_clip();
  }
};

class NestedMetadataStackView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.push_metadata(cgpui::PaintMetadata{
        .opacity = 0.5F,
        .transform = cgpui::AffineTransform::translation(2.0F, 3.0F),
    });
    paint_list.push_metadata(cgpui::PaintMetadata{
        .opacity = 0.5F,
        .transform = cgpui::AffineTransform::translation(4.0F, 5.0F),
    });
    paint_list.fill_rect(
        cgpui::Rect{
            .origin = {.x = 20.0F, .y = 22.0F},
            .size = {.width = 18.0F, .height = 14.0F},
        },
        cgpui::Color{.r = 0.7F, .g = 0.4F, .b = 0.2F, .a = 1.0F});
    paint_list.pop_metadata();
    paint_list.pop_metadata();
  }
};

class ImageOnlyView final : public cgpui::View {
 public:
  ImageOnlyView()
      : asset_(cgpui::ImageAsset{
            .id = cgpui::ImageAssetId{42},
            .logical_size = {.width = 20.0F, .height = 10.0F},
            .bitmap =
                cgpui::DecodedImageBitmap{
                    .width = 2,
                    .height = 2,
                    .stride = 8,
                    .format = cgpui::ImageFormat::rgba8_unorm,
                    .pixels =
                        std::vector<std::uint8_t>{
                            255, 0, 0, 255, 0, 255, 0, 255,
                            0, 0, 255, 255, 255, 255, 255, 255},
                },
        }) {}

  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.push_clip(cgpui::Rect{
        .origin = {.x = 3.0F, .y = 4.0F},
        .size = {.width = 40.0F, .height = 24.0F},
    });
    paint_list.push_metadata(cgpui::PaintMetadata{
        .opacity = 0.625F,
        .transform = cgpui::AffineTransform::translation(5.0F, 7.0F),
    });
    paint_list.draw_image(
        cgpui::Rect{
            .origin = {.x = 8.0F, .y = 9.0F},
            .size = {.width = 20.0F, .height = 10.0F},
        },
        cgpui::describe_image_asset(asset_));
    paint_list.pop_metadata();
    paint_list.pop_clip();
  }

 private:
  cgpui::ImageAsset asset_;
};

int test_runtime_theme_slots_inherit_and_switch_with_invalidation() {
  ThemeFakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  ThemeFakeApplication application(window);
  EmptyView view;
  RecordingFrame frame;
  RecordingRenderer renderer(frame);
  cgpui::WindowRuntime runtime(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&renderer};
      });

  const cgpui::ThemeTokenId accent = cgpui::theme_token("color.accent");
  const cgpui::ThemeTokenId gap = cgpui::theme_token("space.gap");
  const cgpui::ThemeTokenId missing = cgpui::theme_token("missing");
  bool app_theme_invalidated = false;
  bool window_theme_invalidated = false;
  bool clear_theme_invalidated = false;
  bool window_overrode_app_color = false;
  bool window_inherited_app_spacing = false;
  bool clear_fell_back_to_app_color = false;
  bool context_resolved_current_window = false;
  bool missing_tokens_softly_failed = false;

  runtime.set_after_frame_callback([&](const cgpui::WindowRuntimeContext& context) {
    const std::optional<cgpui::Color> context_color =
        context.theme_color(accent);
    const std::optional<float> context_spacing = context.theme_spacing(gap);
    context_resolved_current_window =
        context.window_runtime_id == runtime.root_window_runtime_id() &&
        context_color.has_value() && context_color->r == 10.0F / 255.0F &&
        context_spacing.has_value() && *context_spacing == 8.0F;
  });

  application.on_run = [&] {
    cgpui::Theme app_theme;
    app_theme.set_color(accent, cgpui::rgb(10, 20, 30))
        .set_spacing(gap, cgpui::px(8.0F));
    runtime.set_app_theme(app_theme);
    app_theme_invalidated =
        runtime.invalidation_state().render &&
        runtime.invalidation_state().layout &&
        runtime.invalidation_state().paint &&
        window.request_redraw_count == 1;
    runtime.clear_invalidation();

    cgpui::Theme window_theme;
    window_theme.set_color(accent, cgpui::rgb(40, 50, 60));
    runtime.set_window_theme(runtime.root_window_runtime_id(), window_theme);
    window_theme_invalidated =
        runtime.invalidation_state().render &&
        runtime.invalidation_state().layout &&
        runtime.invalidation_state().paint &&
        window.request_redraw_count == 2;

    const std::optional<cgpui::Color> window_color =
        runtime.theme_color(runtime.root_window_runtime_id(), accent);
    const std::optional<float> inherited_spacing =
        runtime.theme_spacing(runtime.root_window_runtime_id(), gap);
    window_overrode_app_color =
        window_color.has_value() && window_color->r == 40.0F / 255.0F;
    window_inherited_app_spacing =
        inherited_spacing.has_value() && *inherited_spacing == 8.0F;
    missing_tokens_softly_failed =
        !runtime.theme_color(runtime.root_window_runtime_id(), missing)
             .has_value() &&
        !runtime.theme_spacing(runtime.root_window_runtime_id(), missing)
             .has_value();
    runtime.clear_invalidation();

    clear_theme_invalidated =
        runtime.clear_window_theme(runtime.root_window_runtime_id()) &&
        runtime.invalidation_state().render &&
        runtime.invalidation_state().layout &&
        runtime.invalidation_state().paint &&
        window.request_redraw_count == 3;
    const std::optional<cgpui::Color> fallback_color =
        runtime.theme_color(runtime.root_window_runtime_id(), accent);
    clear_fell_back_to_app_color =
        fallback_color.has_value() && fallback_color->r == 10.0F / 255.0F;

    window.emit(cgpui::WindowRedrawRequested{});
  };

  const int result = runtime.run(
      cgpui::WindowDescriptor{
          .title = "Theme Runtime",
          .size = {.width = 320.0F, .height = 240.0F}},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  if (result != 0) {
    return 39;
  }
  if (application.create_window_count != 1 || application.run_count != 1) {
    return 40;
  }
  if (!app_theme_invalidated || !window_theme_invalidated ||
      !clear_theme_invalidated) {
    return 41;
  }
  if (!window_overrode_app_color || !window_inherited_app_spacing ||
      !clear_fell_back_to_app_color) {
    return 42;
  }
  if (!context_resolved_current_window || !missing_tokens_softly_failed) {
    return 43;
  }
  return 0;
}

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

  RecordingFrame wrapped_frame;
  RecordingRenderer wrapped_renderer(wrapped_frame);
  WrappedTextView wrapped_view;
  const auto wrapped_result = cgpui::render_view(
      wrapped_renderer,
      wrapped_view,
      cgpui::Size{64.0F, 64.0F});
  if (!wrapped_result) {
    return 28;
  }
  if (wrapped_frame.text_draw_count != 1 ||
      wrapped_frame.last_text.bounds.origin.x != 2.0F ||
      wrapped_frame.last_text.bounds.origin.y != 4.0F ||
      wrapped_frame.last_text.bounds.size.width != 16.0F ||
      wrapped_frame.last_text.bounds.size.height != 48.0F ||
      wrapped_frame.last_text.lines.size() != 3 ||
      wrapped_frame.last_text.glyphs.size() != 5) {
    return 29;
  }
  if (wrapped_frame.last_text.lines[0].byte_start != 0 ||
      wrapped_frame.last_text.lines[0].byte_end != 2 ||
      wrapped_frame.last_text.lines[1].byte_start != 2 ||
      wrapped_frame.last_text.lines[1].byte_end != 4 ||
      wrapped_frame.last_text.lines[2].byte_start != 4 ||
      wrapped_frame.last_text.lines[2].byte_end != 5) {
    return 30;
  }
  if (wrapped_frame.last_text.glyphs[0].origin.x != 2.0F ||
      wrapped_frame.last_text.glyphs[0].origin.y != 4.0F ||
      wrapped_frame.last_text.glyphs[1].origin.x != 10.0F ||
      wrapped_frame.last_text.glyphs[1].origin.y != 4.0F ||
      wrapped_frame.last_text.glyphs[2].origin.x != 2.0F ||
      wrapped_frame.last_text.glyphs[2].origin.y != 20.0F ||
      wrapped_frame.last_text.glyphs[4].origin.x != 2.0F ||
      wrapped_frame.last_text.glyphs[4].origin.y != 36.0F) {
    return 31;
  }

  if (const int theme_result =
          test_runtime_theme_slots_inherit_and_switch_with_invalidation();
      theme_result != 0) {
    return theme_result;
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

  if (caret.rect.origin.x != 42.0F || caret.byte_offset != 6 ||
      caret.color.a != 1.0F || !caret.clip_rect.has_value() ||
      caret.metadata.opacity != 0.625F) {
    return 18;
  }

  RecordingFrame clip_frame;
  RecordingRenderer clip_renderer(clip_frame);
  NestedClipStackView clip_view;
  cgpui::FrameStatistics clip_stats;
  const auto clip_result = cgpui::render_view(
      clip_renderer,
      clip_view,
      cgpui::Size{128.0F, 64.0F},
      cgpui::DpiScale{},
      &clip_stats);
  if (!clip_result) {
    return 19;
  }
  if (clip_frame.draw_count != 1 || !clip_frame.last_rect.clip_rect ||
      clip_frame.last_rect.clip_rect->origin.x != 8.0F ||
      clip_frame.last_rect.clip_stack.full_depth != 2 ||
      clip_frame.last_rect.clip_stack.clips.size() != 2 ||
      !clip_frame.last_rect.clip_stack.current_clip_rect.has_value() ||
      clip_frame.last_rect.clip_stack.current_clip_rect->size.width != 48.0F) {
    return 20;
  }
  if (clip_stats.clip_stack_command_count != 1 ||
      clip_stats.max_clip_stack_depth != 2) {
    return 21;
  }

  RecordingFrame metadata_frame;
  RecordingRenderer metadata_renderer(metadata_frame);
  NestedMetadataStackView metadata_view;
  cgpui::FrameStatistics metadata_stats;
  const auto metadata_result = cgpui::render_view(
      metadata_renderer,
      metadata_view,
      cgpui::Size{128.0F, 64.0F},
      cgpui::DpiScale{},
      &metadata_stats);
  if (!metadata_result) {
    return 22;
  }
  if (metadata_frame.draw_count != 1 ||
      metadata_frame.last_rect.metadata.opacity != 0.25F ||
      !same_transform(
          metadata_frame.last_rect.metadata.transform,
          cgpui::AffineTransform::translation(6.0F, 8.0F)) ||
      metadata_frame.last_rect.composition_stack.full_depth != 2 ||
      metadata_frame.last_rect.composition_stack.entries.size() != 2 ||
      metadata_frame.last_rect.composition_stack.entries[0].opacity != 0.5F ||
      metadata_frame.last_rect.composition_stack.entries[1].opacity != 0.25F ||
      metadata_frame.last_rect.composition_stack.current_metadata.opacity !=
          0.25F) {
    return 23;
  }
  if (metadata_stats.composition_stack_command_count != 1 ||
      metadata_stats.max_composition_stack_depth != 2) {
    return 24;
  }

  RecordingFrame image_frame;
  RecordingRenderer image_renderer(image_frame);
  ImageOnlyView image_view;
  cgpui::FrameStatistics image_stats;
  const auto image_result = cgpui::render_view(
      image_renderer,
      image_view,
      cgpui::Size{64.0F, 64.0F},
      cgpui::DpiScale{},
      &image_stats);
  if (!image_result) {
    return 67;
  }
  if (image_frame.image_draw_count != 1 || image_frame.draw_count != 0 ||
      image_frame.text_draw_count != 0 ||
      image_stats.submitted_command_count != 1 ||
      image_stats.image_command_count != 1) {
    return 68;
  }
  const cgpui::ImageDraw& image = image_frame.last_image;
  if (image.bounds.origin.x != 8.0F ||
      image.bounds.origin.y != 9.0F ||
      image.bounds.size.width != 20.0F ||
      image.asset.id.value != 42 ||
      image.asset.logical_size.width != 20.0F ||
      image.asset.pixel_width != 2 ||
      image.asset.pixel_height != 2 ||
      image.asset.format != cgpui::ImageFormat::rgba8_unorm ||
      image.asset.byte_size != 16 ||
      !image.clip_rect.has_value() ||
      image.clip_rect->size.width != 40.0F) {
    return 69;
  }
  if (image.metadata.opacity != 0.625F ||
      !same_transform(
          image.metadata.transform,
          cgpui::AffineTransform::translation(5.0F, 7.0F)) ||
      image.clip_stack.full_depth != 1 ||
      image.composition_stack.full_depth != 1) {
    return 70;
  }

  cgpui::TextMeasurementCache text_measurement_cache;
  RecordingFrame cached_text_frame;
  RecordingRenderer cached_text_renderer(cached_text_frame);
  TextOnlyView cached_text_view;
  const auto first_cached_text_result = cgpui::render_view(
      cached_text_renderer,
      cached_text_view,
      cgpui::Size{64.0F, 64.0F},
      cgpui::DpiScale{},
      &text_measurement_cache);
  if (!first_cached_text_result ||
      text_measurement_cache.entry_count() != 1 ||
      text_measurement_cache.miss_count() != 1 ||
      text_measurement_cache.hit_count() != 0) {
    return 25;
  }
  const auto second_cached_text_result = cgpui::render_view(
      cached_text_renderer,
      cached_text_view,
      cgpui::Size{64.0F, 64.0F},
      cgpui::DpiScale{},
      &text_measurement_cache);
  if (!second_cached_text_result ||
      text_measurement_cache.entry_count() != 1 ||
      text_measurement_cache.miss_count() != 1 ||
      text_measurement_cache.hit_count() != 1 ||
      cached_text_frame.text_draw_count != 2 ||
      cached_text_frame.texts[1].glyphs[2].origin.x != 18.0F) {
    return 26;
  }

  cgpui::RendererCommandReport command_report;
  command_report.supported_command_count = metadata_stats.submitted_command_count;
  command_report.submission_plan_record_count = 1;
  command_report.submission_plan_records.push_back(
      cgpui::RendererSubmissionPlanRecord{
          .key =
              cgpui::RendererSubmissionPlanKey{
                  .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
                  .clip_rect = metadata_frame.last_rect.clip_rect,
                  .clip_stack = metadata_frame.last_rect.clip_stack,
              },
          .pipeline =
              cgpui::TextSamplerPipelineDescriptor{
                  .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
              },
          .command_count = metadata_stats.submitted_command_count,
          .command_indices = {0},
      });
  const cgpui::RendererFrameReport frame_report =
      cgpui::renderer_frame_report_from_command_report(command_report);
  return frame_report.supported_primitive_count == 1 &&
                 frame_report.submission_plan_record_count == 1 &&
                 frame_report.gap_count == 0
             ? 0
             : 27;
}

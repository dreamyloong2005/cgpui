#include "cgpui/ui/ui.hpp"

#include <memory>

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override { clear_count += 1; }

  void draw_rect(const cgpui::SolidRect& rect) override {
    draw_count += 1;
    last_rect = rect;
  }

  cgpui::Result<void> present() override {
    present_count += 1;
    return {};
  }

  int clear_count = 0;
  int draw_count = 0;
  int present_count = 0;
  cgpui::SolidRect last_rect;
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
    paint_list.fill_text(
        cgpui::Rect{.origin = {2.0F, 4.0F},
                    .size = {.width = 24.0F, .height = 16.0F}},
        cgpui::Color{.r = 0.8F, .g = 0.9F, .b = 1.0F, .a = 1.0F},
        "abc");
  }
};

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

  return frame.draw_count == 0 ? 0 : 5;
}

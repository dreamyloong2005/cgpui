#include "cgpui/ui/runtime_rendering.hpp"

#include <memory>
#include <vector>

namespace {

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override { order.push_back(1); }
  void draw_rect(const cgpui::SolidRect&) override {}
  void upload_image(const cgpui::ImageAsset& image) override {
    order.push_back(3);
    uploaded_id = image.id;
    uploaded_bytes = image.bitmap.pixels.size();
  }
  void invalidate_image(cgpui::ImageAssetId id) override {
    order.push_back(2);
    invalidated_id = id;
  }
  void draw_image(const cgpui::ImageDraw& image) override {
    order.push_back(4);
    drawn_id = image.asset.id;
  }
  cgpui::Result<void> present() override {
    order.push_back(5);
    return {};
  }

  std::vector<int> order;
  cgpui::ImageAssetId uploaded_id;
  cgpui::ImageAssetId invalidated_id;
  cgpui::ImageAssetId drawn_id;
  std::size_t uploaded_bytes = 0;
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
    void upload_image(const cgpui::ImageAsset& image) override {
      frame_.upload_image(image);
    }
    void invalidate_image(cgpui::ImageAssetId id) override {
      frame_.invalidate_image(id);
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

class ImageAssetUpdateView final : public cgpui::View {
 public:
  ImageAssetUpdateView()
      : asset_(cgpui::ImageAsset{
            .id = cgpui::ImageAssetId{41},
            .logical_size = {.width = 2.0F, .height = 1.0F},
            .bitmap = cgpui::DecodedImageBitmap{
                .width = 2,
                .height = 1,
                .stride = 8,
                .pixels = {255, 0, 0, 255, 0, 255, 0, 255},
            },
        }) {}

  void paint(cgpui::PaintList& paint, cgpui::Size) override {
    paint.invalidate_image(asset_.id);
    paint.upload_image(asset_);
    paint.draw_image(
        {.origin = {}, .size = asset_.logical_size},
        cgpui::describe_image_asset(asset_));
  }

 private:
  cgpui::ImageAsset asset_;
};

} // namespace

int main() {
  cgpui::PaintList reusable;
  cgpui::ImageAsset stale{
      .id = cgpui::ImageAssetId{39},
      .logical_size = {.width = 1.0F, .height = 1.0F},
      .bitmap = {.width = 1, .height = 1, .stride = 4,
                 .pixels = {0, 0, 0, 255}},
  };
  reusable.invalidate_image(stale.id);
  reusable.upload_image(stale);
  reusable.clear();
  if (!reusable.image_invalidations().empty() ||
      !reusable.image_uploads().empty()) return 4;

  RecordingFrame frame;
  RecordingRenderer renderer(frame);
  ImageAssetUpdateView view;
  const auto result = cgpui::render_view(renderer, view, {20.0F, 10.0F});
  if (!result || frame.order != std::vector<int>({1, 2, 3, 4, 5})) return 1;
  if (frame.uploaded_id != cgpui::ImageAssetId{41} ||
      frame.invalidated_id != cgpui::ImageAssetId{41} ||
      frame.drawn_id != cgpui::ImageAssetId{41}) return 2;
  return frame.uploaded_bytes == 8 ? 0 : 3;
}

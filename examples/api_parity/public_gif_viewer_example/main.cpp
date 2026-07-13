#include "cgpui/prelude.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

cgpui::Rect contain_rect(cgpui::Size viewport, cgpui::Size content) {
  if (content.width <= 0.0F || content.height <= 0.0F) return {};
  const float scale = std::min(
      viewport.width / content.width,
      viewport.height / content.height);
  const cgpui::Size size{
      .width = content.width * scale,
      .height = content.height * scale,
  };
  return {
      .origin = {
          .x = (viewport.width - size.width) * 0.5F,
          .y = (viewport.height - size.height) * 0.5F,
      },
      .size = size,
  };
}

class PublicGifViewerExampleView final : public cgpui::View {
 public:
  PublicGifViewerExampleView(
      std::shared_ptr<const cgpui::AssetSource> source,
      std::string path)
      : source_(std::move(source)), path_(std::move(path)) {}

  cgpui::IntoElement render(
      cgpui::Context<PublicGifViewerExampleView>& context) override {
    if (!started_) {
      started_ = true;
      begin_load(context.async_context());
    }
    return cgpui::into_element(
        cgpui::div().size(context.viewport_size).bg(cgpui::rgb(18, 20, 24)));
  }

  void paint(cgpui::PaintList& paint, cgpui::Size viewport) override {
    if (frames_.empty()) return;
    const cgpui::ImageAsset& frame = frames_[frame_index_];
    if (invalidate_pending_) paint.invalidate_image(frame.id);
    if (upload_pending_) paint.upload_image(frame);
    paint.draw_image(
        contain_rect(viewport, frame.logical_size),
        cgpui::describe_image_asset(frame));
    invalidate_pending_ = false;
    upload_pending_ = false;
  }

 private:
  void begin_load(const cgpui::AsyncContextCapability& async) {
    const auto key = reloads_.key(
        source_id_, path_, cgpui::AssetCacheKind::animated_gif);
    if (!key) return;
    auto submission = cgpui::load_asset_async(
        async,
        {.source = source_, .key = *key, .priority = cgpui::TaskPriority::high},
        [this](const cgpui::ViewContext& context,
               cgpui::AsyncAssetLoadResult result) {
          if (!result.bytes || !result.bytes->has_value()) return;
          cgpui::GifDecodeResult decoded =
              cgpui::decode_gif(result.bytes->value().bytes);
          if (!decoded.ready() || decoded.gif.frames.empty()) return;
          loop_ = decoded.gif.loop;
          durations_.clear();
          frames_.clear();
          durations_.reserve(decoded.gif.frames.size());
          frames_.reserve(decoded.gif.frames.size());
          const cgpui::Size logical_size{
              .width = static_cast<float>(decoded.gif.width),
              .height = static_cast<float>(decoded.gif.height),
          };
          for (auto& decoded_frame : decoded.gif.frames) {
            durations_.push_back(std::max(decoded_frame.duration_ms, 1U));
            frames_.push_back(cgpui::ImageAsset{
                .id = image_id_,
                .logical_size = logical_size,
                .bitmap = std::move(decoded_frame.bitmap),
            });
          }
          frame_index_ = 0;
          completed_repeats_ = 0;
          upload_pending_ = true;
          context.request_render();
          schedule_next(context.async_context());
        });
    if (submission) load_ = *submission;
  }

  void schedule_next(const cgpui::AsyncContextCapability& async) {
    if (frames_.size() < 2 || frame_index_ >= durations_.size()) return;
    frame_timer_ = async.schedule_timer(
        durations_[frame_index_],
        [this](const cgpui::ViewContext& context) {
          if (frame_index_ + 1 < frames_.size()) {
            ++frame_index_;
          } else {
            if (!loop_.infinite && completed_repeats_ >= loop_.repeat_count) {
              return;
            }
            ++completed_repeats_;
            frame_index_ = 0;
          }
          invalidate_pending_ = true;
          upload_pending_ = true;
          context.request_render();
          schedule_next(context.async_context());
        });
  }

  static constexpr cgpui::AssetSourceId source_id_{2};
  static constexpr cgpui::ImageAssetId image_id_{202};
  std::shared_ptr<const cgpui::AssetSource> source_;
  std::string path_;
  cgpui::AssetReloadState reloads_;
  cgpui::TaskHandle load_;
  cgpui::TimerId frame_timer_;
  cgpui::GifLoopBehavior loop_;
  std::vector<cgpui::ImageAsset> frames_;
  std::vector<std::uint32_t> durations_;
  std::size_t frame_index_ = 0;
  std::uint16_t completed_repeats_ = 0;
  bool started_ = false;
  bool invalidate_pending_ = false;
  bool upload_pending_ = false;
};

static_assert(cgpui::Render<PublicGifViewerExampleView>);

std::string environment_or(const char* name, const char* fallback) {
  const char* value = std::getenv(name);
  return value == nullptr ? fallback : value;
}

} // namespace

int main() {
  if (std::getenv("CGPUI_RUN_PUBLIC_GIF_VIEWER_EXAMPLE") == nullptr) return 0;
  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }
  auto source = std::make_shared<cgpui::FileAssetSource>(
      std::filesystem::path(environment_or("CGPUI_GIF_ASSET_ROOT", ".")));
  PublicGifViewerExampleView view(
      std::move(source),
      environment_or("CGPUI_GIF_ASSET_PATH", "black-cat-typing.gif"));
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI GIF Viewer")
                       .size(640.0F, 480.0F)
                       .resizable(true)
                       .to_descriptor();
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

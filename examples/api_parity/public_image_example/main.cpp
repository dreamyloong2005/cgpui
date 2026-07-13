#include "cgpui/prelude.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace {

struct ReloadImageAction {
  static constexpr std::string_view name = "image.reload";
};

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

class PublicImageExampleView final : public cgpui::View {
 public:
  PublicImageExampleView(
      std::shared_ptr<const cgpui::AssetSource> source,
      std::string path)
      : source_(std::move(source)), path_(std::move(path)) {}

  cgpui::IntoElement render(
      cgpui::Context<PublicImageExampleView>& context) override {
    context.register_action<ReloadImageAction>(
        [this](const cgpui::ViewContext& action_context) {
          begin_load(action_context.async_context(), true);
          return cgpui::EventResult::consumed_event();
        });
    if (!started_) {
      started_ = true;
      begin_load(context.async_context(), false);
    }
    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .bg(cgpui::rgb(20, 23, 27))
            .on_click([](const cgpui::ElementEventContext& event) {
              return event.dispatch_action(ReloadImageAction::name);
            }));
  }

  void paint(cgpui::PaintList& paint, cgpui::Size viewport) override {
    const auto assets = registry_.raster_assets();
    if (assets.empty()) return;
    const cgpui::ImageAsset& image = assets.front();
    if (invalidate_pending_) paint.invalidate_image(image.id);
    if (upload_pending_) paint.upload_image(image);
    paint.draw_image(
        contain_rect(viewport, image.logical_size),
        cgpui::describe_image_asset(image));
    invalidate_pending_ = false;
    upload_pending_ = false;
  }

 private:
  void begin_load(const cgpui::AsyncContextCapability& async, bool reload) {
    if (load_.active()) (void)load_.cancel();
    if (reload && !reloads_.invalidate(source_id_, path_)) return;
    const auto key = reloads_.key(
        source_id_, path_, cgpui::AssetCacheKind::raster_image);
    if (!key) return;
    auto submission = cgpui::load_asset_async(
        async,
        {.source = source_, .key = *key, .priority = cgpui::TaskPriority::high},
        [this, reload](const cgpui::ViewContext& context,
                       cgpui::AsyncAssetLoadResult result) {
          if (!result.bytes || !result.bytes->has_value()) return;
          cgpui::ImageDecodeResult decoded =
              cgpui::decode_image(result.bytes->value().bytes);
          if (!decoded.ready()) return;
          registry_ = cgpui::ImageAssetRegistry{image_id_};
          (void)registry_.register_image(cgpui::ImageAsset{
              .id = image_id_,
              .logical_size = {
                  .width = static_cast<float>(decoded.bitmap.width),
                  .height = static_cast<float>(decoded.bitmap.height),
              },
              .bitmap = std::move(decoded.bitmap),
          });
          invalidate_pending_ = reload;
          upload_pending_ = true;
          context.request_render();
        });
    if (submission) load_ = *submission;
  }

  static constexpr cgpui::AssetSourceId source_id_{1};
  static constexpr cgpui::ImageAssetId image_id_{101};
  std::shared_ptr<const cgpui::AssetSource> source_;
  std::string path_;
  cgpui::AssetReloadState reloads_;
  cgpui::ImageAssetRegistry registry_{image_id_};
  cgpui::TaskHandle load_;
  bool started_ = false;
  bool invalidate_pending_ = false;
  bool upload_pending_ = false;
};

static_assert(cgpui::Action<ReloadImageAction>);
static_assert(cgpui::Render<PublicImageExampleView>);

std::string environment_or(const char* name, const char* fallback) {
  const char* value = std::getenv(name);
  return value == nullptr ? fallback : value;
}

} // namespace

int main() {
  if (std::getenv("CGPUI_RUN_PUBLIC_IMAGE_EXAMPLE") == nullptr) return 0;
  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }
  auto source = std::make_shared<cgpui::FileAssetSource>(
      std::filesystem::path(environment_or("CGPUI_IMAGE_ASSET_ROOT", ".")));
  PublicImageExampleView view(
      std::move(source),
      environment_or("CGPUI_IMAGE_ASSET_PATH", "image.png"));
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Image Example")
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

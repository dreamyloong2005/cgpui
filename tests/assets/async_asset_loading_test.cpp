#include "cgpui/ui/async_asset_loading.hpp"
#include "window_runtime_test_support.hpp"

#include <atomic>
#include <concepts>
#include <memory>
#include <thread>

static_assert(requires(const cgpui::AsyncContextCapability& context) {
  { context.valid() } -> std::same_as<bool>;
});

namespace {

class RecordingAssetSource final : public cgpui::AssetSource {
 public:
  cgpui::Result<std::optional<cgpui::AssetBytes>> load(
      std::string_view path) const override {
    load_thread = std::this_thread::get_id();
    load_count.fetch_add(1);
    if (path != "assets/payload.bin") return std::nullopt;
    return cgpui::AssetBytes{.bytes = {1, 3, 5, 7}};
  }

  cgpui::Result<std::vector<std::string>> list(
      std::string_view) const override {
    return std::vector<std::string>{};
  }

  mutable std::atomic_size_t load_count = 0;
  mutable std::thread::id load_thread;
};

class AsyncAssetView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  std::thread::id runtime_thread;
  cgpui::WindowRuntimeId expected_runtime_id;
  cgpui::TaskHandle task;
  bool empty_completion_rejected = false;
  bool invalid_key_rejected = false;
  bool submitted = false;
  bool completed = false;
  bool callback_on_runtime_thread = false;
  bool key_matches = false;
  bool bytes_match = false;
  bool bounded_pool_observed = false;
};

cgpui::WindowRuntime* asset_runtime = nullptr;
AsyncAssetView* asset_view = nullptr;
std::shared_ptr<RecordingAssetSource> asset_source;

bool wait_until(const auto& predicate) {
  for (int attempt = 0; attempt < 2000; ++attempt) {
    if (predicate()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return false;
}

void dispatch_async_asset_sequence() {
  auto& runtime = *asset_runtime;
  auto& view = *asset_view;
  view.runtime_thread = std::this_thread::get_id();
  const auto initial = runtime.diagnostics_snapshot();

  view.task = runtime.spawn_task([&](const cgpui::WindowRuntimeContext& cx) {
    view.expected_runtime_id = cx.window_runtime_id;
    auto empty_completion = cgpui::load_asset_async(
        cx.async_context(),
        cgpui::AsyncAssetLoadRequest{
            .source = asset_source,
            .key = cgpui::AssetCacheKey{
                .source_id = cgpui::AssetSourceId{7},
                .path = "assets/payload.bin",
                .kind = cgpui::AssetCacheKind::bytes,
                .revision = 4}},
        {});
    view.empty_completion_rejected = !empty_completion &&
        empty_completion.error().code == cgpui::ErrorCode::invalid_argument;
    if (!view.empty_completion_rejected) {
      if (empty_completion) (void)empty_completion->cancel();
      return;
    }
    auto invalid = cgpui::load_asset_async(
        cx.async_context(),
        cgpui::AsyncAssetLoadRequest{
            .source = asset_source,
            .key = cgpui::AssetCacheKey{
                .source_id = {},
                .path = "assets/payload.bin",
                .kind = cgpui::AssetCacheKind::bytes,
                .revision = 4}},
        [](const cgpui::WindowRuntimeContext&,
           cgpui::AsyncAssetLoadResult) {});
    view.invalid_key_rejected = !invalid &&
        invalid.error().code == cgpui::ErrorCode::invalid_argument;
    if (!view.invalid_key_rejected) {
      if (invalid) (void)invalid->cancel();
      return;
    }
    const auto key = cgpui::make_asset_cache_key(
        cgpui::AssetSourceId{7}, "assets//./payload.bin",
        cgpui::AssetCacheKind::bytes, 4);
    if (!key) return;
    auto submission = cgpui::load_asset_async(
        cx.async_context(),
        cgpui::AsyncAssetLoadRequest{
            .source = asset_source,
            .key = *key,
            .priority = cgpui::TaskPriority::high},
        [&view](const cgpui::WindowRuntimeContext& completion_cx,
                cgpui::AsyncAssetLoadResult result) {
          view.completed = true;
          view.callback_on_runtime_thread =
              std::this_thread::get_id() == view.runtime_thread &&
              completion_cx.window_runtime_id == view.expected_runtime_id;
          view.key_matches = result.key.path == "assets/payload.bin" &&
              result.key.revision == 4;
          view.bytes_match = result.bytes && result.bytes->has_value() &&
              result.bytes->value().bytes ==
                  std::vector<std::uint8_t>({1, 3, 5, 7});
        });
    view.submitted = submission.has_value();
    if (submission) view.task = *submission;
  });
  if (!runtime.complete_task(view.task.id())) return;
  runtime.drain_task_completions();
  if (!view.empty_completion_rejected || !view.invalid_key_rejected) return;

  const bool worker_finished = wait_until([&] {
    return asset_source->load_count.load() == 1 &&
        runtime.diagnostics_snapshot().task_pool_completed_work_count ==
            initial.task_pool_completed_work_count + 1;
  });
  view.bounded_pool_observed = worker_finished &&
      initial.task_pool_worker_count > 0 &&
      asset_source->load_thread != view.runtime_thread;
  runtime.drain_task_completions();
}

bool rejects_empty_source_before_context_access() {
  const auto key = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{9}, "missing-source.bin",
      cgpui::AssetCacheKind::bytes, 0);
  if (!key) return false;
  const auto result = cgpui::load_asset_async(
      cgpui::AsyncContextCapability{},
      cgpui::AsyncAssetLoadRequest{.source = {}, .key = *key},
      [](const cgpui::WindowRuntimeContext&,
         cgpui::AsyncAssetLoadResult) {});
  return !result && result.error().code == cgpui::ErrorCode::invalid_argument;
}

bool rejects_empty_context() {
  const auto key = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{10}, "missing-context.bin",
      cgpui::AssetCacheKind::bytes, 0);
  if (!key) return false;
  const auto result = cgpui::load_asset_async(
      cgpui::AsyncContextCapability{},
      {.source = std::make_shared<RecordingAssetSource>(), .key = *key},
      [](const cgpui::WindowRuntimeContext&,
         cgpui::AsyncAssetLoadResult) {});
  return !result && result.error().code == cgpui::ErrorCode::invalid_argument;
}

} // namespace

int main() {
  if (!rejects_empty_source_before_context_access()) return 5;
  if (!rejects_empty_context()) return 6;
  RuntimeFixture fixture;
  AsyncAssetView view;
  asset_source = std::make_shared<RecordingAssetSource>();
  fixture.app.on_run = &dispatch_async_asset_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app, view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  asset_runtime = &runtime;
  asset_view = &view;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  asset_runtime = nullptr;
  asset_view = nullptr;
  asset_source.reset();
  if (result != 0 || !view.empty_completion_rejected ||
      !view.invalid_key_rejected || !view.submitted) {
    return 1;
  }
  if (!view.completed || !view.callback_on_runtime_thread) return 2;
  if (!view.key_matches || !view.bytes_match) return 3;
  if (!view.bounded_pool_observed || !view.task.complete()) return 4;
  return 0;
}

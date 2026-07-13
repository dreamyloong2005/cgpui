#include "cgpui/ui/async_asset_loading.hpp"
#include "window_runtime_test_support.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <thread>

namespace {

class OutcomeAssetSource final : public cgpui::AssetSource {
 public:
  cgpui::Result<std::optional<cgpui::AssetBytes>> load(
      std::string_view path) const override {
    load_count.fetch_add(1);
    if (path == "missing.bin") return std::nullopt;
    if (path == "error.bin") {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::asset_io_failed,
          .message = "scripted asset read failure",
      });
    }
    if (path == "throw.bin") {
      throw std::runtime_error("scripted source exception");
    }
    if (path == "blocked.bin") {
      blocked_started.store(true);
      while (!release_blocked.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      return cgpui::AssetBytes{.bytes = {9}};
    }
    return std::nullopt;
  }

  cgpui::Result<std::vector<std::string>> list(
      std::string_view) const override {
    return std::vector<std::string>{};
  }

  mutable std::atomic_size_t load_count = 0;
  mutable std::atomic_bool blocked_started = false;
  mutable std::atomic_bool release_blocked = false;
};

class OutcomeView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::TaskHandle missing;
  cgpui::TaskHandle error;
  cgpui::TaskHandle throwing;
  cgpui::TaskHandle cancelled;
  bool submitted = false;
  bool missing_matches = false;
  bool error_matches = false;
  bool throwing_matches = false;
  bool cancelled_completion_ran = false;
  bool source_owned_during_work = false;
  bool source_released_after_work = false;
};

cgpui::WindowRuntime* outcome_runtime = nullptr;
OutcomeView* outcome_view = nullptr;
std::weak_ptr<OutcomeAssetSource> outcome_source;

bool wait_until(const auto& predicate) {
  for (int attempt = 0; attempt < 500; ++attempt) {
    if (predicate()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return false;
}

cgpui::AssetCacheKey key_for(std::string path) {
  return *cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{12}, path, cgpui::AssetCacheKind::bytes, 0);
}

void dispatch_outcome_sequence() {
  auto& runtime = *outcome_runtime;
  auto& view = *outcome_view;
  const auto initial = runtime.diagnostics_snapshot();
  auto source = std::make_shared<OutcomeAssetSource>();
  outcome_source = source;

  const cgpui::TaskHandle setup = runtime.spawn_task(
      [&](const cgpui::WindowRuntimeContext& cx) {
        auto missing = cgpui::load_asset_async(
            cx.async_context(),
            {.source = source, .key = key_for("missing.bin")},
            [&](const cgpui::WindowRuntimeContext&,
                cgpui::AsyncAssetLoadResult result) {
              view.missing_matches = result.bytes &&
                  !result.bytes->has_value() &&
                  result.key.path == "missing.bin";
            });
        auto error = cgpui::load_asset_async(
            cx.async_context(),
            {.source = source, .key = key_for("error.bin")},
            [&](const cgpui::WindowRuntimeContext&,
                cgpui::AsyncAssetLoadResult result) {
              view.error_matches = !result.bytes &&
                  result.bytes.error().code ==
                      cgpui::ErrorCode::asset_io_failed;
            });
        auto cancelled = cgpui::load_asset_async(
            cx.async_context(),
            {.source = source, .key = key_for("blocked.bin")},
            [&](const cgpui::WindowRuntimeContext&,
                cgpui::AsyncAssetLoadResult) {
              view.cancelled_completion_ran = true;
            });
        auto throwing = cgpui::load_asset_async(
            cx.async_context(),
            {.source = source, .key = key_for("throw.bin")},
            [&](const cgpui::WindowRuntimeContext&,
                cgpui::AsyncAssetLoadResult result) {
              view.throwing_matches = !result.bytes &&
                  result.bytes.error().code ==
                      cgpui::ErrorCode::asset_io_failed &&
                  result.bytes.error().message.find("source exception") !=
                      std::string::npos;
            });
        view.submitted = missing && error && cancelled && throwing;
        if (!view.submitted) return;
        view.missing = *missing;
        view.error = *error;
        view.cancelled = *cancelled;
        view.throwing = *throwing;
      });
  if (!runtime.complete_task(setup.id())) return;
  runtime.drain_task_completions();
  if (!view.submitted) return;
  source.reset();

  if (!wait_until([&] {
        auto active = outcome_source.lock();
        return active && active->blocked_started.load();
      })) {
    return;
  }
  view.source_owned_during_work = !outcome_source.expired();
  const bool cancelled = view.cancelled.cancel();
  if (auto active = outcome_source.lock()) active->release_blocked.store(true);
  if (!cancelled || !wait_until([&] {
        return runtime.diagnostics_snapshot().task_pool_completed_work_count >=
            initial.task_pool_completed_work_count + 4;
      })) {
    return;
  }
  runtime.drain_task_completions();
  view.source_released_after_work = wait_until([&] {
    return outcome_source.expired();
  });
}

} // namespace

int main() {
  RuntimeFixture fixture;
  OutcomeView view;
  fixture.app.on_run = &dispatch_outcome_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app, view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  outcome_runtime = &runtime;
  outcome_view = &view;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  outcome_runtime = nullptr;
  outcome_view = nullptr;
  outcome_source.reset();
  if (result != 0 || !view.submitted) return 1;
  if (!view.missing_matches || !view.error_matches || !view.throwing_matches) {
    return 2;
  }
  if (!view.cancelled.cancelled() || view.cancelled_completion_ran) return 3;
  if (!view.missing.complete() || !view.error.complete() ||
      !view.throwing.complete()) return 4;
  return view.source_owned_during_work && view.source_released_after_work ? 0 : 5;
}

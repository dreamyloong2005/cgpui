#include "cgpui/cgpui.hpp"

#include <atomic>
#include <chrono>
#include <expected>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

namespace {

static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>().try_spawn_task(
                  std::declval<cgpui::TaskCompletionCallback>())),
              cgpui::Result<cgpui::TaskHandle>>);
static_assert(
    std::is_same_v<
        decltype(std::declval<cgpui::WindowRuntime&>()
                     .try_spawn_background_task(
                         std::declval<cgpui::BackgroundTaskCallback>(),
                         std::declval<cgpui::TaskCompletionCallback>())),
        cgpui::Result<cgpui::TaskHandle>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntimeContext&>()
                           .try_spawn_task(
                               std::declval<cgpui::TaskCompletionCallback>())),
              cgpui::Result<cgpui::TaskHandle>>);
static_assert(
    std::is_same_v<
        decltype(std::declval<cgpui::WindowRuntimeContext&>()
                     .try_spawn_background_task(
                         std::declval<cgpui::BackgroundTaskCallback>(),
                         std::declval<cgpui::TaskCompletionCallback>())),
        cgpui::Result<cgpui::TaskHandle>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::AsyncContextCapability>()
                           .try_spawn_task(
                               std::declval<cgpui::TaskCompletionCallback>())),
              cgpui::Result<cgpui::TaskHandle>>);
static_assert(
    std::is_same_v<
        decltype(std::declval<cgpui::AsyncContextCapability>()
                     .try_spawn_background_task(
                         std::declval<cgpui::BackgroundTaskCallback>(),
                         std::declval<cgpui::TaskCompletionCallback>())),
        cgpui::Result<cgpui::TaskHandle>>);

class AsyncResultFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class AsyncResultRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<AsyncResultFrame>();
  }
};

class AsyncResultView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class AsyncResultWindow final : public cgpui::PlatformWindow {
 public:
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }

  cgpui::WindowState state() const override {
    return cgpui::WindowState{
        .framebuffer_size = {.width = 320.0F, .height = 240.0F},
        .scale = cgpui::DpiScale{1.0F},
        .close_requested = false};
  }

  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}
};

class AsyncResultApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&,
      cgpui::PlatformEventCallback) override {
    return std::unexpected(cgpui::Error{
        .code = cgpui::ErrorCode::window_creation_failed,
        .message = "window creation is outside this test"});
  }

  int run() override { return 0; }
  void quit() override {}
};

struct AsyncResultFixture {
  AsyncResultApplication app;
  AsyncResultView view;
  AsyncResultRenderer renderer;
  AsyncResultWindow window;
  cgpui::WindowRuntime runtime{
      app,
      view,
      [this](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &renderer; }};

  [[nodiscard]] cgpui::WindowRuntimeContext context() {
    return cgpui::WindowRuntimeContext{
        .runtime = runtime,
        .application = app,
        .platform_window = window,
        .renderer = renderer,
        .window_runtime_id = runtime.root_window_runtime_id(),
        .view_id = cgpui::ViewId{1},
        .viewport_size = {.width = 320.0F, .height = 240.0F},
        .scale = cgpui::DpiScale{1.0F},
        .input = {},
        .event_route = {},
        .last_event_result = {},
        .last_event_dispatch = {},
        .frame_index = 0};
  }
};

int test_invalid_foreground_task_returns_error_without_task_record() {
  AsyncResultFixture fixture;
  const cgpui::RuntimeDiagnosticsSnapshot before =
      fixture.runtime.diagnostics_snapshot();

  cgpui::Result<cgpui::TaskHandle> result =
      fixture.runtime.try_spawn_task(cgpui::TaskCompletionCallback{});

  if (result.has_value()) {
    return 1;
  }
  if (result.error().code != cgpui::ErrorCode::invalid_argument ||
      result.error().message !=
          "foreground task completion callback is required") {
    return 2;
  }
  const cgpui::RuntimeDiagnosticsSnapshot after =
      fixture.runtime.diagnostics_snapshot();
  if (after.task_count != before.task_count ||
      after.active_task_count != before.active_task_count ||
      after.queued_task_count != before.queued_task_count) {
    return 3;
  }
  if (fixture.runtime.spawn_task(cgpui::TaskCompletionCallback{}).id().value !=
      0) {
    return 4;
  }
  return 0;
}

int test_invalid_background_task_returns_error_without_task_record() {
  AsyncResultFixture fixture;

  cgpui::Result<cgpui::TaskHandle> missing_work =
      fixture.runtime.try_spawn_background_task(
          cgpui::BackgroundTaskCallback{},
          [](const cgpui::WindowRuntimeContext&) {});
  cgpui::Result<cgpui::TaskHandle> missing_completion =
      fixture.runtime.try_spawn_background_task(
          [](cgpui::TaskCancellationToken) {},
          cgpui::TaskCompletionCallback{});

  if (missing_work.has_value() || missing_completion.has_value()) {
    return 5;
  }
  if (missing_work.error().code != cgpui::ErrorCode::invalid_argument ||
      missing_work.error().message != "background task work is required") {
    return 6;
  }
  if (missing_completion.error().code != cgpui::ErrorCode::invalid_argument ||
      missing_completion.error().message !=
          "background task completion callback is required") {
    return 7;
  }
  const cgpui::RuntimeDiagnosticsSnapshot diagnostics =
      fixture.runtime.diagnostics_snapshot();
  if (diagnostics.task_count != 0 || diagnostics.background_task_count != 0) {
    return 8;
  }
  if (fixture.runtime
          .spawn_background_task(
              cgpui::BackgroundTaskCallback{},
              [](const cgpui::WindowRuntimeContext&) {})
          .id()
          .value != 0) {
    return 9;
  }
  return 0;
}

int test_contexts_forward_async_spawn_results() {
  AsyncResultFixture fixture;
  cgpui::WindowRuntimeContext context = fixture.context();
  const cgpui::AsyncContextCapability async_context =
      context.async_context();

  int completion_count = 0;
  cgpui::Result<cgpui::TaskHandle> foreground =
      context.try_spawn_task([&](const cgpui::WindowRuntimeContext&) {
        completion_count += 1;
      });

  if (!foreground.has_value() || foreground->id().value == 0 ||
      !fixture.runtime.complete_task(foreground->id())) {
    return 10;
  }
  fixture.runtime.drain_task_completions();
  if (!foreground->complete() || completion_count != 1) {
    return 11;
  }

  std::atomic<int> worker_count = 0;
  int background_completion_count = 0;
  cgpui::Result<cgpui::TaskHandle> background =
      async_context.try_spawn_background_task(
          [&](cgpui::TaskCancellationToken token) {
            if (!token.cancellation_requested()) {
              worker_count.fetch_add(1);
            }
          },
          [&](const cgpui::WindowRuntimeContext&) {
            background_completion_count += 1;
          });

  if (!background.has_value() || background->id().value == 0) {
    return 12;
  }
  for (int attempt = 0; attempt < 200 && background->active(); ++attempt) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  fixture.runtime.drain_task_completions();
  if (!background->complete() || worker_count.load() != 1 ||
      background_completion_count != 1) {
    return 13;
  }

  cgpui::Result<cgpui::TaskHandle> missing_from_context =
      context.try_spawn_task(cgpui::TaskCompletionCallback{});
  cgpui::Result<cgpui::TaskHandle> missing_from_async_context =
      async_context.try_spawn_background_task(
          [](cgpui::TaskCancellationToken) {},
          cgpui::TaskCompletionCallback{});
  if (missing_from_context.has_value() ||
      missing_from_async_context.has_value()) {
    return 14;
  }
  if (missing_from_context.error().code !=
          cgpui::ErrorCode::invalid_argument ||
      missing_from_async_context.error().code !=
          cgpui::ErrorCode::invalid_argument) {
    return 15;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result =
          test_invalid_foreground_task_returns_error_without_task_record();
      result != 0) {
    return result;
  }
  if (const int result =
          test_invalid_background_task_returns_error_without_task_record();
      result != 0) {
    return result;
  }
  if (const int result = test_contexts_forward_async_spawn_results();
      result != 0) {
    return result;
  }
  return 0;
}

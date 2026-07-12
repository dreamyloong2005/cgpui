#include "cgpui/ui/async_io_hook.hpp"
#include "window_runtime_test_support.hpp"

#include <array>
#include <cstddef>
#include <thread>
#include <vector>

namespace {

class AsyncIoView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  std::thread::id runtime_thread;
  std::vector<int> completion_order;
  std::vector<std::byte> normal_payload;
  cgpui::Error failure{
      .code = cgpui::ErrorCode::invalid_argument,
      .message = {}};
  bool callbacks_on_runtime_thread = true;
  bool creation_surfaces_match = false;
  bool state_matches = false;
};

cgpui::WindowRuntime* async_io_runtime = nullptr;
RuntimeFixture* async_io_fixture = nullptr;
AsyncIoView* async_io_view = nullptr;

cgpui::AsyncIoCompletionCallback completion(int order) {
  return [order](
             const cgpui::WindowRuntimeContext& context,
             const cgpui::AsyncIoResult& result) {
    auto& view = *async_io_view;
    view.callbacks_on_runtime_thread =
        view.callbacks_on_runtime_thread &&
        std::this_thread::get_id() == view.runtime_thread;
    view.completion_order.push_back(order);
    if (order == 2 && result.has_value()) {
      view.normal_payload = *result;
      cgpui::AsyncIoHook context_hook = context.create_async_io_hook(
          [](const cgpui::WindowRuntimeContext&,
             const cgpui::AsyncIoResult&) {});
      cgpui::AsyncIoHook capability_hook =
          context.async_context().create_async_io_hook(
              cgpui::TaskPriority::high,
              [](const cgpui::WindowRuntimeContext&,
                 const cgpui::AsyncIoResult&) {});
      view.creation_surfaces_match =
          context_hook.id().value != 0 && capability_hook.id().value != 0 &&
          context_hook.cancel() && capability_hook.cancel();
    }
    if (order == 3 && !result.has_value()) {
      view.failure = result.error();
    }
  };
}

void dispatch_async_io_sequence() {
  auto& runtime = *async_io_runtime;
  auto& fixture = *async_io_fixture;
  auto& view = *async_io_view;
  view.runtime_thread = std::this_thread::get_id();

  cgpui::AsyncIoHook low = runtime.create_async_io_hook(
      cgpui::TaskPriority::low, completion(1));
  cgpui::AsyncIoHook normal = runtime.create_async_io_hook(completion(2));
  cgpui::AsyncIoHook high = runtime.create_async_io_hook(
      cgpui::TaskPriority::high, completion(3));
  cgpui::AsyncIoHook cancelled = runtime.create_async_io_hook(completion(4));

  const std::array<std::byte, 2> payload{
      std::byte{0x21}, std::byte{0x42}};
  bool low_notified = false;
  bool normal_notified = false;
  bool high_notified = false;
  std::thread low_thread([&] { low_notified = low.notify({}); });
  std::thread normal_thread([&] {
    normal_notified = normal.notify(
        std::vector<std::byte>(payload.begin(), payload.end()));
  });
  std::thread high_thread([&] {
    high_notified = high.notify_error(cgpui::Error{
        .code = cgpui::ErrorCode::platform_initialization_failed,
        .message = "read failed"});
  });
  low_thread.join();
  normal_thread.join();
  high_thread.join();

  const bool cancelled_once = cancelled.cancel();
  const bool duplicate_suppressed =
      !normal.notify({}) && !high.notify({}) &&
      !cancelled.notify({}) && !cancelled.cancel();
  const bool pending_before_wakeup = view.completion_order.empty();
  fixture.app.dispatch_wakeup();

  view.state_matches =
      low_notified && normal_notified && high_notified && cancelled_once &&
      duplicate_suppressed && pending_before_wakeup &&
      fixture.app.request_wakeup_count >= 3 &&
      view.completion_order == std::vector<int>({3, 2, 1}) &&
      view.creation_surfaces_match &&
      view.normal_payload ==
          std::vector<std::byte>(payload.begin(), payload.end()) &&
      view.failure.code ==
          cgpui::ErrorCode::platform_initialization_failed &&
      view.failure.message == "read failed" &&
      low.complete() && normal.complete() && high.complete() &&
      cancelled.cancelled() && !cancelled.active();
}

} // namespace

int main() {
  RuntimeFixture fixture;
  AsyncIoView view;
  fixture.app.on_run = &dispatch_async_io_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  async_io_runtime = &runtime;
  async_io_fixture = &fixture;
  async_io_view = &view;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  async_io_runtime = nullptr;
  async_io_fixture = nullptr;
  async_io_view = nullptr;
  if (result != 0) return 1;
  if (!view.callbacks_on_runtime_thread) return 2;
  if (!view.state_matches) return 3;

  cgpui::AsyncIoHook detached;
  {
    RuntimeFixture teardown_fixture;
    AsyncIoView teardown_view;
    cgpui::WindowRuntime teardown_runtime(
        teardown_fixture.app,
        teardown_view,
        [&](const cgpui::RenderSurfaceDescriptor&) {
          return cgpui::Result<cgpui::Renderer*>{&teardown_fixture.renderer};
        });
    detached = teardown_runtime.create_async_io_hook(completion(5));
  }
  return detached.cancelled() && !detached.notify({}) ? 0 : 4;
}

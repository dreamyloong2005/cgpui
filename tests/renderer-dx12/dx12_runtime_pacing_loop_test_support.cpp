#include "dx12_runtime_pacing_test_support.hpp"
#include <cstdlib>
#include <string_view>
#include <utility>

namespace cgpui_test {
bool Dx12RuntimePacingFixture::require(bool condition, const char* message) {
  if (condition) return true;
  if (!failed) std::cerr << "DX12 native-runtime synthetic-admission: " << message << '\n';
  failed = true;
  application.quit();
  return false;
}
unsigned Dx12RuntimePacingFixture::index(cgpui::WindowRuntimeId id) const {
  return id == ids[0] ? 0 : id == ids[1] ? 1 : 2;
}
bool Dx12RuntimePacingFixture::pending_without_success() const {
  for (const auto& window : windows) {
    if (!window.pending || window.paints || window.successes || window.captures || window.errors ||
        window.api.polls != window.begins || window.pending != window.begins) return false;
  }
  return true;
}
cgpui::Result<std::unique_ptr<cgpui::Renderer>> Dx12RuntimePacingFixture::create(
    const cgpui::RenderSurfaceDescriptor& descriptor) {
  const unsigned slot = factories++;
  if (slot >= windows.size()) return std::unexpected(cgpui::Error{
      .code = cgpui::ErrorCode::renderer_initialization_failed,
      .message = "pacing runtime unexpectedly recreated a renderer"});
  return dx12_runtime_pacing_capture(descriptor, windows[slot]);
}
void Dx12RuntimePacingFixture::setup(cgpui::AppContext& context) {
  auto& runtime = context.runtime;
  ids[0] = runtime.root_window_runtime_id();
  runtime.set_error_callback([this](const cgpui::Error& error) {
    ++errors;
    (void)require(false, error.message.c_str());
  });
  runtime.set_after_frame_callback([this](const auto& frame) { after_frame(frame); });
  runtime.set_after_event_callback([this](const auto& event_context, const cgpui::EventDispatchRecord& event) {
    const unsigned slot = index(event_context.window_runtime_id);
    if (!require(slot < windows.size(), "event came from unknown runtime window")) return;
    if (event.event_kind == cgpui::EventKind::window_close_requested) ++windows[slot].closes;
    if (event.event_kind == cgpui::EventKind::keyboard_key) ++windows[slot].key_routes;
  });
  auto child = runtime.try_open_window(cgpui::WindowOptions{}.title("DX12 pending child")
      .size(320, 200).backdrop(cgpui::WindowBackdrop::none),
      std::make_unique<Dx12RuntimePacingScene>(windows[1]));
  if (!require(child.has_value(), "native child creation failed")) return;
  ids[1] = child->runtime_id;
  const auto timer = runtime.schedule_repeating_timer(5, [this](const auto& timer_context) {
    ++user_ticks;
    if (!require(Clock::now() - started < std::chrono::seconds(5), "native runtime phase deadline exceeded")) return;
    if (!failed && !completed && on_tick) on_tick(timer_context);
  });
  (void)require(timer.value != 0, "independent user timer creation failed");
}
void Dx12RuntimePacingFixture::after_frame(const cgpui::WindowRuntimeContext& context) {
  const unsigned slot = index(context.window_runtime_id);
  if (!require(slot < windows.size(), "successful frame came from unknown runtime window")) return;
  auto& window = windows[slot];
  const auto info = context.renderer.backend_info();
  const char* selected = std::getenv("CGPUI_DX12_ADAPTER");
  const bool warp = selected && std::string_view(selected) == "warp";
  if (!require(info.backend == cgpui::RendererBackend::dx12 &&
      info.selection == cgpui::RendererBackendSelection::dx12 && info.fallback_reason.empty() &&
      info.adapter_luid == window.initial.adapter_luid && info.adapter_luid.has_value() &&
      info.device_generation == window.initial.device_generation && (!warp || info.software_adapter) &&
      window.api.forwarded.sets == 1 && window.api.forwarded.gets == 1 &&
      GetWindowThreadProcessId(window.hwnd, nullptr) == GetCurrentThreadId() &&
      window.api.waited.status == WAIT_OBJECT_0 && !window.errors &&
      dx12_runtime_pacing_pixels(context, window) && pacing_begin_debug_clean(info),
      "successful native frame identity/pixels/resize/debug invalid")) return;
  ++window.successes;
  if (on_frame) on_frame(context);
}
int Dx12RuntimePacingFixture::run(const char* title) {
  Dx12RuntimePacingScene root(windows[0]);
  cgpui::AppRunnerOptions options;
  options.window.title = title;
  options.window.size = {320, 200};
  options.window.backdrop = cgpui::WindowBackdrop::none;
  options.setup_context = [this](auto& context) { setup(context); };
  const int result = cgpui::run_app(application, root,
      [this](const auto& descriptor) { return create(descriptor); }, std::move(options));
  if (!require(result == 0 && !failed && !errors && completed && factories == 2 && user_ticks > 1,
      "native loop did not finish all required phases")) return 1;
  for (const auto& window : windows) {
    if (!require(window.api.forwarded.closes == 1 && window.api.forwarded.close_ok &&
        !window.errors && window.pending && window.api.forwarded.sets == 1 && window.api.forwarded.gets == 1,
        "native runtime final handle/pending/error invariants invalid")) return 1;
  }
  std::cout << "PACING_RUNTIME synthetic-wait native-platform/renderer root-pending=" << windows[0].pending
            << " child-pending=" << windows[1].pending << " user-ticks=" << user_ticks << '\n';
  return 0;
}
} // namespace cgpui_test

#include "runtime_frame_readiness_test_support.hpp"

#include <iostream>
#include <utility>

namespace readiness_test {
namespace {
class Window final : public cgpui::PlatformWindow {
 public:
  Window(Application& application, std::shared_ptr<State> state,
         std::unique_ptr<cgpui::PlatformWindow> native)
      : application_(application), state_(std::move(state)), native_(std::move(native)) {
    state_->native = dynamic_cast<cgpui::detail::TestPlatformWindow*>(native_.get());
  }
  ~Window() override { state_->native = nullptr; state_->destroyed = true; }
  cgpui::NativeSurfaceHandle native_surface() const override { return native_->native_surface(); }
  cgpui::WindowState state() const override { return native_->state(); }
  void request_redraw() override { ++state_->redraws; application_.enqueue(state_); }
  void request_close() override { application_.emit(state_, cgpui::WindowCloseRequested{}); }
  void set_title(std::string_view title) override { native_->set_title(title); }
  void set_cursor(cgpui::CursorShape cursor) override { native_->set_cursor(cursor); }
  void set_window_control_regions(std::vector<cgpui::WindowControlRegion> regions) override {
    ++state_->control_region_publications;
    native_->set_window_control_regions(std::move(regions));
  }
  void set_ime_text_input_placement(std::optional<cgpui::ImeTextInputPlacement> value) override {
    native_->set_ime_text_input_placement(std::move(value));
  }
 private:
  Application& application_;
  std::shared_ptr<State> state_;
  std::unique_ptr<cgpui::PlatformWindow> native_;
};

class Frame final : public cgpui::RenderFrame {
 public:
  explicit Frame(std::shared_ptr<State> state) : state_(std::move(state)) {}
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override {
    ++state_->presents;
    if (state_->present_error)
      return std::unexpected(cgpui::Error{*state_->present_error, "scripted present outcome"});
    return {};
  }
 private:
  std::shared_ptr<State> state_;
};
} // namespace

cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> Application::create_window(
    const cgpui::WindowDescriptor& descriptor, cgpui::PlatformEventCallback callback) {
  auto native = delegate_.create_window(descriptor, std::move(callback));
  if (!native) return std::unexpected(native.error());
  root = last_created = std::make_shared<State>();
  return std::make_unique<Window>(*this, root, std::move(*native));
}
cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> Application::create_child_window(
    const cgpui::WindowDescriptor& descriptor, cgpui::PlatformWindow& parent,
    cgpui::PlatformEventCallback callback) {
  auto native = delegate_.create_child_window(descriptor, parent, std::move(callback));
  if (!native) return std::unexpected(native.error());
  last_created = std::make_shared<State>();
  children.push_back(last_created);
  return std::make_unique<Window>(*this, last_created, std::move(*native));
}
std::uint64_t Application::monotonic_time_ms() const { return now; }
void Application::request_wakeup() { ++wake_requests; wake_pending_ = true; }
void Application::request_wakeup_after(std::uint64_t delay) {
  ++timer_requests;
  wake_deadline = now + delay;
  auto callback = std::exchange(on_next_timer_request, {});
  if (callback) callback(); // Optional one-shot transport reentrancy, no timer policy.
}
void Application::cancel_wakeup_after() { wake_deadline.reset(); }
int Application::run() { if (on_run) on_run(); return 0; }
void Application::quit() { ++quits; }
void Application::enqueue(const std::shared_ptr<State>& state) { redraw_queue_.push_back(state); }
void Application::emit(const std::shared_ptr<State>& state, const cgpui::PlatformEvent& event) {
  if (state && state->native) state->native->dispatch_event(event);
}
void Application::advance(std::uint64_t milliseconds) { now += milliseconds; }
bool Application::pump() {
  // Transport only: bounded event dispatch at the controlled clock, not runtime
  // timer selection, deadline computation, pending policy, or frame retry logic.
  for (unsigned turn = 0; turn != 256; ++turn) {
    if (wake_deadline && *wake_deadline <= now) { wake_deadline.reset(); wake_pending_ = true; }
    if (wake_pending_) {
      wake_pending_ = false;
      emit(root, cgpui::WindowWakeupRequested{});
      continue;
    }
    if (redraw_queue_.empty()) return true;
    auto window = redraw_queue_.front().lock();
    redraw_queue_.pop_front();
    if (window) emit(window, cgpui::WindowRedrawRequested{});
  }
  return false; // An immediate retry/self-wake loop cannot pass silently.
}

View::View(std::shared_ptr<State> state) : state_(std::move(state)) {}
void View::bind(std::shared_ptr<State> state) { state_ = std::move(state); }
void View::paint(cgpui::PaintList&, cgpui::Size) { ++state_->paints; }
Renderer::Renderer(std::shared_ptr<State> state) : state_(std::move(state)) {}
cgpui::Result<void> Renderer::resize(cgpui::Size, cgpui::DpiScale) { ++state_->resizes; return {}; }
cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> Renderer::begin_frame() {
  ++state_->begins;
  if (!state_->outcomes.empty()) {
    const auto outcome = state_->outcomes.front();
    state_->outcomes.pop_front();
    if (outcome) return std::unexpected(cgpui::Error{*outcome, "scripted begin outcome"});
  }
  return std::make_unique<Frame>(state_);
}

Fixture::Fixture()
    : root_scene(std::make_shared<State>()), root_view(root_scene),
      runtime(app, root_view, [this](const auto&) -> cgpui::Result<cgpui::Renderer*> {
        if (app.last_created == app.root) root_view.bind(app.root);
        renderers.push_back(std::make_unique<Renderer>(app.last_created));
        return renderers.back().get();
      }) {
  runtime.set_after_frame_callback([this](const auto&) { ++after_frames; });
  runtime.set_error_callback([this](const auto& error) {
    ++errors; last_error = error.code; last_error_message = error.message;
  });
}
std::shared_ptr<State> Fixture::open_child() {
  auto scene = std::make_shared<State>();
  auto view = std::make_unique<View>(scene);
  auto* actual_view = view.get();
  auto opened = runtime.try_open_window(cgpui::WindowOptions{}.size(100, 100), std::move(view));
  if (!require(opened.has_value(), "child creation failed")) return {};
  actual_view->bind(app.last_created);
  ids_.push_back({app.last_created, opened->runtime_id});
  return app.last_created;
}
cgpui::WindowRuntimeId Fixture::id(const std::shared_ptr<State>& state) const {
  if (state == app.root) return runtime.root_window_runtime_id();
  for (const auto& [candidate, id] : ids_) if (candidate.lock() == state) return id;
  return {};
}
void Fixture::paint(const std::shared_ptr<State>& state) {
  app.emit(state, cgpui::WindowRedrawRequested{}); // An external OS-style paint.
}
int Fixture::run(std::function<void()> script) {
  app.on_run = std::move(script);
  return runtime.run({}, {.request_initial_redraw = false});
}
bool Fixture::require(bool condition, const char* reason) {
  if (!condition && !failure) { failure = 1; std::cerr << "frame readiness: " << reason << '\n'; }
  return condition;
}

} // namespace readiness_test

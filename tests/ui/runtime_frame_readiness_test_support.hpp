#pragma once

#include "cgpui/cgpui.hpp"
#include "test_app_internal.hpp"

#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace readiness_test {

struct State {
  cgpui::detail::TestPlatformWindow* native = nullptr;
  unsigned redraws = 0, begins = 0, paints = 0, presents = 0, resizes = 0;
  unsigned control_region_publications = 0;
  bool destroyed = false;
  std::deque<std::optional<cgpui::ErrorCode>> outcomes;
  std::optional<cgpui::ErrorCode> present_error;
  std::deque<std::optional<cgpui::ErrorCode>> resize_outcomes;
  cgpui::Size last_resize_size{};
  cgpui::DpiScale last_resize_scale{};
  std::function<void()> on_present;
};

// Only transport/time are adapted. Window events are dispatched by the existing
// TestPlatformWindow; scheduling/retry algorithms belong to actual WindowRuntime.
class Application final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&, cgpui::PlatformEventCallback) override;
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor&, cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback) override;
  std::uint64_t monotonic_time_ms() const override;
  void request_wakeup() override;
  void request_wakeup_after(std::uint64_t) override;
  void cancel_wakeup_after() override;
  int run() override;
  void quit() override;
  void enqueue(const std::shared_ptr<State>&);
  void emit(const std::shared_ptr<State>&, const cgpui::PlatformEvent&);
  void advance(std::uint64_t milliseconds);
  bool pump();

  std::function<void()> on_run, on_next_timer_request;
  std::shared_ptr<State> root, last_created;
  std::vector<std::shared_ptr<State>> children;
  std::uint64_t now = 0;
  unsigned quits = 0, wake_requests = 0, timer_requests = 0;
  std::optional<std::uint64_t> wake_deadline;

 private:
  cgpui::detail::TestPlatformApplication delegate_;
  std::deque<std::weak_ptr<State>> redraw_queue_;
  bool wake_pending_ = false;
};

class View final : public cgpui::View {
 public:
  explicit View(std::shared_ptr<State> state);
  void bind(std::shared_ptr<State> state);
  void paint(cgpui::PaintList&, cgpui::Size) override;
 private:
  std::shared_ptr<State> state_;
};

class Renderer final : public cgpui::Renderer {
 public:
  explicit Renderer(std::shared_ptr<State> state);
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override;
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override;
 private:
  std::shared_ptr<State> state_;
};

class Fixture {
 public:
  Fixture();
  std::shared_ptr<State> open_child();
  cgpui::WindowRuntimeId id(const std::shared_ptr<State>&) const;
  void paint(const std::shared_ptr<State>&);
  int run(std::function<void()> script);
  bool require(bool condition, const char* reason);

  Application app;
  std::shared_ptr<State> root_scene;
  View root_view;
  std::vector<std::unique_ptr<Renderer>> renderers;
  cgpui::WindowRuntime runtime;
  unsigned after_frames = 0, errors = 0;
  std::optional<cgpui::ErrorCode> last_error;
  std::string last_error_message;
  int failure = 0;

 private:
  std::vector<std::pair<std::weak_ptr<State>, cgpui::WindowRuntimeId>> ids_;
};

} // namespace readiness_test

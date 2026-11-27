#include "runtime_window_lifecycle_test_support.hpp"

#include <array>
#include <iostream>

namespace {
using namespace runtime_window_test;

struct LifetimeProbe {
  cgpui::WindowRuntime* runtime = nullptr;
  cgpui::WindowRuntimeId id;
  std::shared_ptr<NativeState> native;
  Counters* counters = nullptr;
  bool static_mode = false;
  bool static_frame_completed = false;
  bool close_on_event = false;
  bool event_returned_alive = false;
  unsigned frame_complete_order = 0;
  unsigned tree_destroy_order = 0;
  unsigned view_destroy_order = 0;
  int close_attempts = 0;
  int close_successes = 0;
  int failure = 0;

  void close() {
    ++close_attempts;
    if (cgpui::close_popup(*runtime, id)) ++close_successes;
    else failure = 1;
  }
};

class ClosingElement final : public cgpui::Element {
 public:
  explicit ClosingElement(LifetimeProbe& probe) : probe_(probe) {}
  ~ClosingElement() override {
    probe_.tree_destroy_order = ++probe_.counters->sequence;
    // Report premature destruction deterministically instead of inducing UAF in
    // the broken installer by erasing the host while its reset() is executing.
    if (!probe_.static_frame_completed || probe_.native->presents != 2) {
      probe_.failure = 2;
      return;
    }
    probe_.close();
    if (probe_.view_destroy_order != 0 || probe_.native->destroyed) probe_.failure = 3;
  }

 private:
  LifetimeProbe& probe_;
};

class SwitchingView final : public cgpui::View {
 public:
  explicit SwitchingView(LifetimeProbe& probe) : probe_(probe) {}
  ~SwitchingView() override {
    probe_.view_destroy_order = ++probe_.counters->sequence;
  }
  bool supports_static_render() const override { return probe_.static_mode; }
  cgpui::AnyElement render(cgpui::ViewContext&) override {
    return std::make_unique<ClosingElement>(probe_);
  }
  cgpui::StaticElementTreeView render_static(cgpui::ViewContext&) override {
    const cgpui::StaticElementTreeView tree{nodes_, children_, cgpui::ElementId{1}};
    if (!tree.valid()) probe_.failure = 4;
    return tree;
  }
  void paint(cgpui::PaintList&, cgpui::Size) override {}
  cgpui::EventResult handle_event(const cgpui::PlatformEvent&,
                                 const cgpui::WindowRuntimeContext&) override {
    if (!probe_.close_on_event) return cgpui::EventResult::unhandled();
    probe_.close();
    probe_.event_returned_alive = probe_.view_destroy_order == 0 && !probe_.native->destroyed;
    return cgpui::EventResult::consumed_event();
  }

 private:
  LifetimeProbe& probe_;
  std::array<cgpui::StaticElementNode, 2> nodes_{
      cgpui::StaticElementNode{.id = {1}, .children = {0, 1},
                               .bounds = {{0, 0}, {120, 80}}},
      cgpui::StaticElementNode{.id = {2}, .parent_id = {1},
                               .bounds = {{10, 10}, {80, 40}}, .focusable = true}};
  std::array<cgpui::ElementId, 1> children_{cgpui::ElementId{2}};
};

int self_close_after_static_transition(bool close_on_event) {
  Application app;
  View root;
  LifetimeProbe probe;
  probe.counters = &app.counters;
  probe.static_mode = close_on_event;
  probe.close_on_event = close_on_event;
  app.on_run = [&] {
    const auto opened = probe.runtime->try_open_window(
        popup_options(), std::make_unique<SwitchingView>(probe));
    if (!opened) { probe.failure = 5; return; }
    probe.id = opened->runtime_id;
    probe.native = app.children.back();
    probe.runtime->set_after_frame_callback([&](const auto& context) {
      if (context.window_runtime_id != probe.id || !probe.static_mode) return;
      if (probe.tree_destroy_order != 0 || probe.view_destroy_order != 0) probe.failure = 6;
      probe.static_frame_completed = true;
      probe.frame_complete_order = ++app.counters.sequence;
    });
    probe.native->emit(cgpui::WindowRedrawRequested{});
    if (!close_on_event) {
      if (probe.native->presents != 1 || probe.tree_destroy_order != 0) {
        probe.failure = 7;
        return;
      }
      probe.static_mode = true;
      probe.native->emit(cgpui::WindowRedrawRequested{});
    } else {
      probe.native->emit(key());
    }
    if (probe.failure != 0) return;
    if (!probe.static_frame_completed || probe.close_attempts != 1 ||
        probe.close_successes != 1 || probe.view_destroy_order == 0 ||
        probe.native->destroyed || probe.native->destroyed_during_callback) {
      probe.failure = 8;
      return;
    }
    if (close_on_event ? !probe.event_returned_alive
                       : probe.tree_destroy_order <= probe.frame_complete_order ||
                             probe.tree_destroy_order >= probe.view_destroy_order) {
      probe.failure = 9;
      return;
    }
    app.wake();
    app.wake();
    if (!probe.native->destroyed || probe.native->destroyed_during_callback ||
        probe.native->renderer_destroyed_after_window || probe.native->live_renderers != 0 ||
        probe.native->renderer_destroy_order >= probe.native->window_destroy_order ||
        probe.runtime->window_runtime_record(probe.id) != nullptr) probe.failure = 10;
  };
  cgpui::AppRunnerOptions options;
  options.runtime.request_initial_redraw = false;
  options.setup = [&](auto& runtime) { probe.runtime = &runtime; };
  const int result = cgpui::run_app(app, root, [&](const auto&)
      -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
    return std::make_unique<Renderer>(app.last_created);
  }, options);
  return result != 0 ? 11 : probe.failure;
}
} // namespace

int main() {
  for (const bool close_on_event : {false, true}) {
    if (const int result = self_close_after_static_transition(close_on_event); result != 0) {
      std::cerr << "static window lifetime " << (close_on_event ? "event" : "tree")
                << " failure: " << result << '\n';
      return result;
    }
  }
  return 0;
}

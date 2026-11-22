#include "runtime_window_lifecycle_test_support.hpp"

#include <iostream>

namespace {
using namespace runtime_window_test;

class ReentrantRenderer final : public cgpui::Renderer {
 public:
  ReentrantRenderer(std::shared_ptr<NativeState> native, std::function<void()> callback)
      : renderer_(std::move(native)), on_destroy_(std::move(callback)) {}
  ~ReentrantRenderer() override { if (on_destroy_) on_destroy_(); }
  bool popup_windows_supported() const override { return true; }
  cgpui::Result<void> resize(cgpui::Size size, cgpui::DpiScale scale) override {
    return renderer_.resize(size, scale);
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return renderer_.begin_frame();
  }
 private:
  runtime_window_test::Renderer renderer_;
  std::function<void()> on_destroy_;
};

bool renderer_before_native(const NativeState& state) {
  return state.destroyed && !state.destroyed_during_callback &&
      !state.renderer_destroyed_after_window && state.live_renderers == 0 &&
      state.renderer_destroy_order != 0 &&
      state.renderer_destroy_order < state.window_destroy_order;
}

cgpui::AppRunnerOptions setup(cgpui::WindowRuntime*& runtime) {
  cgpui::AppRunnerOptions options;
  options.runtime.request_initial_redraw = false;
  options.setup = [&runtime](auto& value) { runtime = &value; };
  return options;
}

int renderer_destructor_closes_another_window() {
  Application app;
  View root;
  cgpui::WindowRuntime* runtime = nullptr;
  cgpui::WindowRuntimeId second_id;
  int reentrant_closes = 0;
  int failure = 0;
  app.on_run = [&] {
    const auto first = runtime->try_open_window(popup_options(), std::make_unique<View>());
    const auto second = runtime->try_open_window(popup_options(), std::make_unique<View>());
    if (!first || !second) { failure = 1; return; }
    second_id = second->runtime_id;
    if (!cgpui::close_popup(*runtime, first->runtime_id)) { failure = 2; return; }
    app.wake();
    app.wake();
    if (reentrant_closes != 1 || !runtime->app_opened_windows().empty() ||
        app.counters.live_renderers != 1 ||
        !renderer_before_native(*app.children[0]) ||
        !renderer_before_native(*app.children[1])) failure = 3;
  };
  const int result = cgpui::run_app(app, root, [&](const auto&)
      -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
    std::function<void()> on_destroy;
    if (app.children.size() == 1) {
      on_destroy = [&] {
        ++reentrant_closes;
        if (!cgpui::close_popup(*runtime, second_id)) failure = 4;
      };
    }
    return std::make_unique<ReentrantRenderer>(app.last_created, std::move(on_destroy));
  }, setup(runtime));
  if (result != 0 || failure != 0) return failure != 0 ? failure : 5;
  return app.counters.destroyed_renderers == 3 && renderer_before_native(*app.root) ? 0 : 6;
}

struct TreeLifetime {
  unsigned tree_destroy_order = 0;
  unsigned view_destroy_order = 0;
};

class DestructionElement final : public cgpui::Element {
 public:
  DestructionElement(Counters& counters, TreeLifetime& lifetime, std::function<void()> callback)
      : counters_(counters), lifetime_(lifetime), on_destroy_(std::move(callback)) {}
  ~DestructionElement() override {
    lifetime_.tree_destroy_order = ++counters_.sequence;
    if (on_destroy_) on_destroy_();
  }
 private:
  Counters& counters_;
  TreeLifetime& lifetime_;
  std::function<void()> on_destroy_;
};

class TreeView final : public cgpui::View {
 public:
  TreeView(Counters& counters, TreeLifetime& lifetime, std::function<void()> callback = {})
      : counters_(counters), lifetime_(lifetime), on_tree_destroy_(std::move(callback)) {}
  ~TreeView() override { lifetime_.view_destroy_order = ++counters_.sequence; }
  cgpui::AnyElement render(cgpui::ViewContext&) override {
    return std::make_unique<DestructionElement>(counters_, lifetime_, on_tree_destroy_);
  }
  void paint(cgpui::PaintList&, cgpui::Size) override {}
 private:
  Counters& counters_;
  TreeLifetime& lifetime_;
  std::function<void()> on_tree_destroy_;
};

int tree_destructor_closes_another_window() {
  Application app;
  View root;
  cgpui::WindowRuntime* runtime = nullptr;
  cgpui::WindowRuntimeId second_id;
  TreeLifetime first_lifetime;
  TreeLifetime second_lifetime;
  int reentrant_closes = 0;
  int failure = 0;
  app.on_run = [&] {
    auto close_second = [&] {
      ++reentrant_closes;
      if (!cgpui::close_popup(*runtime, second_id)) failure = 10;
    };
    const auto first = runtime->try_open_window(popup_options(),
        std::make_unique<TreeView>(app.counters, first_lifetime, close_second));
    const auto second = runtime->try_open_window(popup_options(),
        std::make_unique<TreeView>(app.counters, second_lifetime));
    if (!first || !second) { failure = 11; return; }
    second_id = second->runtime_id;
    for (const auto& native : app.children) native->emit(cgpui::WindowRedrawRequested{});
    if (app.children[0]->presents != 1 || app.children[1]->presents != 1 ||
        first_lifetime.tree_destroy_order != 0 || second_lifetime.tree_destroy_order != 0) {
      failure = 12;
      return;
    }
    if (!cgpui::close_popup(*runtime, first->runtime_id)) { failure = 13; return; }
    app.wake();
    app.wake();
    if (reentrant_closes != 1 || !runtime->app_opened_windows().empty()) failure = 14;
    for (const auto& lifetime : {first_lifetime, second_lifetime}) {
      if (lifetime.tree_destroy_order == 0 ||
          lifetime.tree_destroy_order >= lifetime.view_destroy_order) failure = 15;
    }
    for (const auto& native : app.children) {
      if (!renderer_before_native(*native)) failure = 16;
    }
  };
  const int result = cgpui::run_app(app, root, [&](const auto&)
      -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
    return std::make_unique<Renderer>(app.last_created);
  }, setup(runtime));
  return result != 0 ? 17 : failure;
}

int shutdown_renderer_destructor_attempts_to_close_another_window() {
  Application app;
  View root;
  cgpui::WindowRuntime* runtime = nullptr;
  cgpui::WindowRuntimeId second_id;
  int close_attempts = 0;
  int failure = 0;
  app.on_run = [&] {
    const auto first = runtime->try_open_window(popup_options(), std::make_unique<View>());
    const auto second = runtime->try_open_window(popup_options(), std::make_unique<View>());
    if (!first || !second) { failure = 20; return; }
    second_id = second->runtime_id;
    if (app.counters.live_renderers != 3 || app.children[0]->destroyed ||
        app.children[1]->destroyed) failure = 21;
    // Both popup records remain active until run() deactivates the native windows.
  };
  const int result = cgpui::run_app(app, root, [&](const auto&)
      -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
    const auto native = app.last_created;
    const bool first_child = app.children.size() == 1;
    return std::make_unique<ReentrantRenderer>(native, [&, native, first_child] {
      if (native->destroyed) failure = 22;
      if (first_child) {
        ++close_attempts;
        // Shutdown may have already made B inactive; either close result is valid.
        (void)cgpui::close_popup(*runtime, second_id);
      }
      if (native->destroyed) failure = 23;
    });
  }, setup(runtime));
  if (result != 0 || failure != 0) return failure != 0 ? failure : 24;
  if (close_attempts != 1 || app.children.size() != 2 ||
      app.counters.created_renderers != 3 || app.counters.destroyed_renderers != 3 ||
      app.counters.live_renderers != 0 || !renderer_before_native(*app.root)) return 25;
  for (const auto& native : app.children) {
    if (!renderer_before_native(*native)) return 26;
  }
  return 0;
}
} // namespace

int main() {
  for (const auto test : {renderer_destructor_closes_another_window,
                          tree_destructor_closes_another_window,
                          shutdown_renderer_destructor_attempts_to_close_another_window}) {
    if (const int result = test(); result != 0) {
      std::cerr << "reentrant retirement failure: " << result << '\n';
      return result;
    }
  }
  return 0;
}

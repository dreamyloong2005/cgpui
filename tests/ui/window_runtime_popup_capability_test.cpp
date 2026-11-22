#include "popup_test_support.hpp"

#include <iostream>

namespace {

struct Fixture {
  popup_test::Application application;
  int root_alive = 0;
  popup_test::CountingView root_view{root_alive};
  std::vector<std::unique_ptr<popup_test::Renderer>> renderers;
  bool root_support;
  bool child_support;
  cgpui::WindowRuntime runtime;

  Fixture(bool platform, bool root, bool child)
      : root_support(root), child_support(child),
        runtime(application, root_view,
                [this](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
                  renderers.push_back(std::make_unique<popup_test::Renderer>(
                      renderers.empty() ? root_support : child_support));
                  return renderers.back().get();
                }) {
    application.supports_popups = platform;
  }

  int run() {
    return runtime.run({.size = {640, 480}}, {.request_initial_redraw = false});
  }
};

bool unsupported(const cgpui::Result<cgpui::AppOpenedWindow>& result) {
  return !result && result.error().code == cgpui::ErrorCode::unsupported_platform;
}

int absent_renderer() {
  int alive = 0;
  Fixture f(true, true, true);
  if (cgpui::popup_windows_supported(f.runtime)) return 1;
  const auto popup = cgpui::open_popup(f.runtime, popup_test::placement(), {120, 80},
                                     std::make_unique<popup_test::CountingView>(alive));
  const auto direct = f.runtime.try_open_window(
      popup_test::options(), std::make_unique<popup_test::CountingView>(alive));
  if (!unsupported(popup) || !unsupported(direct) || alive != 0 ||
      !f.runtime.app_opened_windows().empty() || !f.application.children.empty()) return 2;
  // Ordinary windows opened before run are still queued for later activation.
  const auto ordinary = f.runtime.try_open_window(cgpui::WindowOptions{}.size(100, 80));
  if (!ordinary) return 3;
  int failure = 0;
  f.application.on_run = [&] {
    const auto* record = f.runtime.window_runtime_record(ordinary->runtime_id);
    if (record == nullptr || !record->active || f.application.children.size() != 1) failure = 4;
  };
  return f.run() == 0 ? failure : 5;
}

int capability_matrix(bool platform, bool renderer) {
  int alive = 0;
  Fixture f(platform, renderer, renderer);
  int failure = 0;
  f.application.on_run = [&] {
    const bool expected = platform && renderer;
    if (cgpui::popup_windows_supported(f.runtime) != expected) { failure = 10; return; }
    const auto popup = cgpui::open_popup(f.runtime, popup_test::placement(), {120, 80},
                                       std::make_unique<popup_test::CountingView>(alive));
    if (expected) {
      if (!popup || alive != 1 || f.application.children.size() != 1 ||
          !f.application.children[0]->open || !cgpui::close_popup(f.runtime, popup->runtime_id) ||
          alive != 0) { failure = 11; return; }
      const auto direct = f.runtime.try_open_window(popup_test::options());
      if (!direct || !cgpui::close_popup(f.runtime, direct->runtime_id)) { failure = 12; return; }
    } else {
      if (!unsupported(popup) || alive != 0) { failure = 13; return; }
      const std::size_t records = f.runtime.window_runtime_records().size();
      const auto direct = f.runtime.try_open_window(
          popup_test::options(), std::make_unique<popup_test::CountingView>(alive));
      const auto without_view = f.runtime.try_open_window(popup_test::options());
      if (!unsupported(direct) || !unsupported(without_view) || alive != 0 ||
          !f.application.children.empty() || !f.runtime.app_opened_windows().empty() ||
          f.runtime.window_runtime_records().size() != records) { failure = 14; return; }
      // The legacy API keeps a failed record and its owned view; it must not
      // activate a native popup or leave the record pointing to a destroyed view.
      const auto legacy = f.runtime.open_window(
          popup_test::options(), std::make_unique<popup_test::CountingView>(alive));
      const auto* record = f.runtime.window_runtime_record(legacy.runtime_id);
      if (record == nullptr || record->active || record->window != nullptr ||
          !record->native_window_error ||
          record->native_window_error->code != cgpui::ErrorCode::unsupported_platform ||
          f.runtime.find_view(legacy.root_view_id) == nullptr || alive != 1 ||
          !f.application.children.empty()) { failure = 15; return; }
      const auto legacy_empty = f.runtime.open_window(popup_test::options());
      record = f.runtime.window_runtime_record(legacy_empty.runtime_id);
      if (record == nullptr || !record->native_window_error || record->active ||
          !f.application.children.empty()) { failure = 16; return; }
    }
    const std::size_t before = f.application.children.size();
    const auto ordinary = f.runtime.try_open_window(cgpui::WindowOptions{}.size(100, 80));
    if (!ordinary || f.application.children.size() != before + 1 ||
        !f.runtime.window_runtime_record(ordinary->runtime_id)->active) failure = 17;
  };
  return f.run() == 0 ? failure : 18;
}

int child_renderer_refusal() {
  int alive = 0;
  Fixture f(true, true, false);
  int failure = 0;
  f.application.on_run = [&] {
    if (!cgpui::popup_windows_supported(f.runtime)) { failure = 20; return; }
    const std::size_t records = f.runtime.window_runtime_records().size();
    const auto popup = cgpui::open_popup(f.runtime, popup_test::placement(), {120, 80},
                                       std::make_unique<popup_test::CountingView>(alive));
    if (!unsupported(popup) || alive != 0 || f.application.children.size() != 1 ||
        f.application.children[0]->open || f.renderers.size() != 2 ||
        !f.runtime.app_opened_windows().empty() ||
        f.runtime.window_runtime_records().size() != records) failure = 21;
  };
  return f.run() == 0 ? failure : 22;
}

} // namespace

int main() {
  for (const auto test : {absent_renderer, child_renderer_refusal}) {
    if (const int result = test(); result != 0) {
      std::cerr << "popup capability test failed: " << result << '\n';
      return result;
    }
  }
  for (const bool platform : {false, true}) {
    for (const bool renderer : {false, true}) {
      if (const int result = capability_matrix(platform, renderer); result != 0) {
        std::cerr << "popup capability matrix " << platform << '/' << renderer
                  << " failed: " << result << '\n';
        return result;
      }
    }
  }
  return 0;
}

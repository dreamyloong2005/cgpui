#include "cgpui/cgpui.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace {
class View final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};
class Frame final : public cgpui::RenderFrame {
 public:
  explicit Frame(int& attempts) : attempts_(attempts) {}
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override {
    if (++attempts_ == 1) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::frame_retry_required,
          .message = "native event-loop retry integration"});
    }
    return {};
  }
 private:
  int& attempts_;
};
class Renderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<Frame>(attempts);
  }
  int attempts = 0;
};
}

int main() {
  auto application = cgpui::create_platform_application();
  if (!application) return 1;
  View view;
  std::vector<std::unique_ptr<Renderer>> renderers;
  cgpui::WindowRuntime runtime(**application, view,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        renderers.push_back(std::make_unique<Renderer>());
        return renderers.back().get();
      });
  cgpui::WindowRuntimeId child;
  bool child_presented = false, open_failed = false, timed_out = false;
  runtime.set_after_frame_callback([&](const cgpui::ViewContext& context) {
    if (context.window_runtime_id == runtime.root_window_runtime_id() && child.value == 0) {
      auto opened = runtime.try_open_window({}, std::make_unique<View>());
      if (!opened) { open_failed = true; (*application)->quit(); return; }
      child = opened->runtime_id;
      cgpui::Window(runtime, child).request_render();
    } else if (context.window_runtime_id == child) {
      child_presented = true;
      (*application)->quit();
    }
  });
  // A missing wakeup must fail rather than hang the native test suite.
  const HANDLE done = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (done == nullptr) return 2;
  const DWORD thread = GetCurrentThreadId();
  std::thread watchdog([&] {
    if (WaitForSingleObject(done, 5000) == WAIT_TIMEOUT) {
      timed_out = true;
      PostThreadMessageW(thread, WM_QUIT, 0, 0);
    }
  });
  const int result = runtime.run({
      .title = "CGPUI native frame retry", .size = {160, 90}});
  SetEvent(done);
  watchdog.join();
  CloseHandle(done);
  // Ordinary expose paints can redraw the root while creating the child;
  // the child's first successful frame must be exactly its second attempt.
  if (result != 0 || timed_out || open_failed || !child_presented ||
      renderers.size() != 2 || renderers[0]->attempts < 2 || renderers[1]->attempts != 2) {
    std::cerr << "native frame retry failed: exit=" << result
              << " timeout=" << timed_out << " child=" << child_presented << '\n';
    return 3;
  }
  return 0;
}

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!stream) throw std::runtime_error(std::string("missing readiness owner: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
bool has(const std::string& source, const char* token) { return source.find(token) != std::string::npos; }
void require(bool value, const char* reason) { if (!value) throw std::runtime_error(reason); }
bool before(const std::string& text, const char* first, const char* second) {
  const auto a = text.find(first), b = text.find(second);
  return a != std::string::npos && b != std::string::npos && a < b;
}
} // namespace

int main() {
  try {
    const auto header = read("src/ui/runtime_frame_readiness_internal.hpp");
    const auto owner = read("src/ui/runtime_frame_readiness.cpp");
    require(std::count(header.begin(), header.end(), '\n') <= 200 &&
                std::count(owner.begin(), owner.end(), '\n') <= 280,
            "readiness lifetime and policy need focused private owners");
    require(has(header, "class WindowRuntime::RuntimeFrameReadiness") &&
                has(header, "TimerId timer") && has(header, "std::uint64_t token") &&
                !has(header, "Renderer*") && !has(header, "PlatformWindow*") &&
                !has(header, "HANDLE") && !has(header, "schedule_window_timer("),
            "readiness declarations must not own native/frame pointers or inline policy");
    require(has(read("src/ui/runtime_state_scheduling_internal.hpp"),
                "RuntimeFrameReadiness frame_readiness_") &&
                has(owner, "runtime.schedule_window_timer(window, delay, false,") &&
                has(owner, "context.request_render()") && has(owner, "pending.token == token") &&
                has(owner, "std::exchange(pending_, {})") &&
                has(owner, "if (!found->delivered) found->timer = timer") &&
                !has(owner, "next_token_ = 0") && !has(owner, "try_draw_frame") &&
                !has(owner, "PostMessage") && !has(owner, "InvalidateRect") &&
                !has(owner, "spawn_background_task") && !has(owner, "WindowRedrawRequested"),
            "readiness must reuse bound timers and ordinary redraw without native dispatch or token reuse");
    require(!has(read("src/ui/runtime_frame_retry.cpp"), "frame_not_ready"),
            "existing immediate recovery retry must not become delayed presentation polling");
    for (const auto* path : {"src/ui/runtime_renderer_frame_results.cpp",
                             "src/ui/runtime_window_frame_results.cpp"}) {
      const auto source = read(path);
      require(before(source, "frame_readiness_.clear(", "runtime_publish_window_control_regions(") &&
                  before(source, "frame_readiness_.clear(", "if (state_->core.after_frame_callback_)"),
              "successful frame must cancel pending wake before platform/user reentrancy");
      const auto continuation = source.substr(source.find("frame_readiness_.clear("));
      require(before(continuation, "state_->core.should_quit_", "runtime_publish_window_control_regions(") &&
                  before(continuation, "state_->core.shutting_down_", "runtime_publish_window_control_regions("),
              "frame success must recheck lifecycle after cancellation can reenter platform callbacks");
      if (has(continuation, "record.runtime_id"))
        require(before(continuation, "!record.active", "runtime_publish_window_control_regions(") &&
                    has(continuation, "record.window == nullptr") && has(continuation, "record.renderer == nullptr"),
                "child success cannot dereference detached resources after timer cancellation");
      else
        require(has(continuation, "state_->windows.window_ == nullptr") &&
                    has(continuation, "state_->windows.renderer_ == nullptr") && has(continuation, "abort_frame_scheduling()"),
                "root success must stop frame scheduling when cancellation reenters lifecycle");
    }
    for (const auto* path : {"src/ui/runtime_window_resource_cleanup.cpp",
                             "src/ui/runtime_window_records.cpp",
                             "src/ui/runtime_window_shutdown.cpp",
                             "src/ui/runtime_run.cpp", "src/ui/runtime_shutdown.cpp",
                             "src/ui/runtime_event_control.cpp"})
      require(has(read(path), "frame_readiness_.clear"), "every root/child/session retirement must clear readiness");
    require(has(read("src/ui/render_view.cpp"), "present violated begin-only frame_not_ready contract") &&
                has(read("include/cgpui/core/error.hpp"), "frame_not_ready,") &&
                has(read("include/cgpui/renderer/renderer_frame.hpp"), "It is begin-only:"),
            "neutral pending must be a documented begin-only result, not a successful/partial present");
    for (const auto* name : {"delay", "cancellation", "lifetime", "fairness", "backoff", "present"}) {
      const std::string file = std::string("window_runtime_frame_readiness_") + name + "_test.cpp";
      require(has(read("tests/ui/mcpp.toml"), file.c_str()), "readiness behavior cases require discovery");
    }
    require(has(read("tests/ui/mcpp.toml"), "runtime_frame_readiness_test_support.cpp") &&
                has(read("tests/architecture/mcpp.toml"), "runtime_frame_readiness_structure_test.cpp") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "runtime_frame_readiness_structure_test|tests/architecture/runtime_frame_readiness_structure_test.cpp"),
            "support and module guards require mcpp and CI registration");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

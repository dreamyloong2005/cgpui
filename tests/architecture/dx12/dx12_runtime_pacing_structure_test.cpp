#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& path) {
  std::ifstream input(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!input) throw std::runtime_error("missing runtime pacing leaf: " + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
bool has(const std::string& source, const char* token) { return source.find(token) != std::string::npos; }
void once(const std::string& source, const char* token) {
  const auto first = source.find(token);
  require(first != std::string::npos && source.find(token, first + 1) == std::string::npos,
      "capture observer must make exactly one real begin call");
}
}

int main() {
  try {
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_runtime_pacing_test_support.hpp", "dx12_runtime_pacing_capture_test_support.cpp",
                            "dx12_runtime_pacing_scene_test_support.cpp", "dx12_runtime_pacing_loop_test_support.cpp",
                            "dx12_runtime_pacing_fairness_test.cpp", "dx12_runtime_pacing_close_test.cpp",
                            "dx12_runtime_pacing_backoff_test.cpp"}) {
      const std::string path = std::string("tests/renderer-dx12/") + name;
      const auto source = read(path);
      const bool header = path.ends_with(".hpp");
      require(std::count(source.begin(), source.end(), '\n') <= (header ? 100 : 200),
          "native runtime pacing support must remain focused");
      if (!header) require(has(manifest, name), "register every native runtime test/support leaf");
      for (const auto* forbidden : {"wait_for_test_frame", "pacing_begin_bounded", "Sleep(", "PeekMessage",
                                    "DispatchMessage", "TestPlatformApplication", "advance_time(", "handle_event_for_window("})
        require(!has(source, forbidden), "native runtime tests must not replace transport/clock or retry/pump begin");
    }
    const auto capture = read("tests/renderer-dx12/dx12_runtime_pacing_capture_test_support.cpp");
    once(capture, "renderer_->begin_frame()");
    require(has(capture, "cgpui::dx12_create_renderer(descriptor, window.api)") &&
        has(capture, "ErrorCode::frame_not_ready") && has(capture, "return std::unexpected(frame.error())") &&
        has(capture, "return std::move(*frame)") && !has(capture, "present()"),
        "observer must return actual native frames/errors unchanged, without a Present interceptor");
    const auto loop = read("tests/renderer-dx12/dx12_runtime_pacing_loop_test_support.cpp");
    require(has(loop, "cgpui::run_app(application, root,") && has(loop, "schedule_repeating_timer(5,") &&
        has(loop, "info.device_generation == window.initial.device_generation") &&
        has(loop, "pacing_begin_debug_clean(info)"), "actual runtime/user timers/native identity must be observed");
    const auto fairness = read("tests/renderer-dx12/dx12_runtime_pacing_fairness_test.cpp");
    require(has(fairness, "create_platform_application()") && has(fairness, "SetWindowPos(") &&
        has(fairness, "PostMessageW(window.hwnd, WM_KEYDOWN") &&
        has(fairness, "windows[0].api.polls >= polls_after_input[0] + 2") &&
        !has(fairness, ".request_render()") && !has(fairness, "->begin_frame()"),
        "fairness must exercise native resize/input and depend on runtime readiness for recovery");
    const auto backoff = read("tests/renderer-dx12/dx12_runtime_pacing_backoff_test.cpp");
    require(has(backoff, "elapsed >= 350") && has(backoff, "elapsed >= 192") &&
        has(backoff, "late-polls=") && has(backoff, "polls > 0 && polls <= maximum") &&
        has(backoff, "fixture.user_ticks > ticks_before + 1") &&
        !has(backoff, ".request_render()") && !has(backoff, "->begin_frame()"),
        "quiet-interval backoff must observe native timers without forcing rendering");
    const auto close = read("tests/renderer-dx12/dx12_runtime_pacing_close_test.cpp");
    require(has(close, "create_platform_application()") && has(close, "PostMessageW(child.hwnd, WM_CLOSE") &&
        has(close, "PostMessageW(root.hwnd, WM_CLOSE") && has(close, "milliseconds(120)") &&
        has(close, "child.api.polls == closed_polls") && !has(close, "application.quit()"),
        "pending-close test must use actual close delivery and observe stale timer suppression");
    require(has(read("tests/architecture/mcpp.toml"), "\"dx12/*_structure_test.cpp\"") &&
        has(read("scripts/ci/architecture-header-targets.txt"),
            "dx12_runtime_pacing_structure_test|tests/architecture/dx12/dx12_runtime_pacing_structure_test.cpp"),
        "register native runtime pacing architecture locally and in CI");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

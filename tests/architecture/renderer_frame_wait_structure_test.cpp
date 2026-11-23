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
  if (!stream) throw std::runtime_error(std::string("missing frame-wait module: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool condition, const char* reason) { if (!condition) throw std::runtime_error(reason); }
std::string section(const std::string& text, const char* name) {
  const auto start = text.find(name);
  require(start != std::string::npos, "missing manifest section");
  const auto end = text.find("\n[", start + 1);
  return text.substr(start, end == std::string::npos ? end : end - start);
}
} // namespace

int main() {
  try {
    const auto header = read("tests/support/frame_wait_test_support.hpp");
    const auto internal = read("tests/support/frame_wait_test_support_internal.hpp");
    const auto owner = read("tests/support/frame_wait_test_support.cpp");
    const auto native = read("tests/support/frame_wait_win32_test_support.cpp");
    require(std::count(header.begin(), header.end(), '\n') <= 50 &&
                std::count(internal.begin(), internal.end(), '\n') <= 50 &&
                std::count(owner.begin(), owner.end(), '\n') <= 90 &&
                std::count(native.begin(), native.end(), '\n') <= 50,
            "test-only frame acquisition needs focused leaf modules");
    require(has(header, "std::thread::id message_owner") && !has(header, "FrameWaitHooks") &&
                !has(header, "windows.h") && !has(header, "HWND") && !has(header, "Dx12") &&
                has(internal, "struct FrameWaitHooks") && !has(internal, "windows.h"),
            "public test helper must expose only an explicit owner policy, not native/callback internals");
    require(has(owner, "return wait_for_test_frame(renderer, {});") &&
                has(owner, "policy.message_owner == std::this_thread::get_id()") &&
                has(owner, "if (may_pump && hooks.pump) hooks.pump(hooks.context, 32)") &&
                has(owner, "std::chrono::seconds(3)") && has(owner, "std::chrono::milliseconds(1)") &&
                has(owner, "std::this_thread::sleep_for(delay)") &&
                has(owner, "frame.error().code != cgpui::ErrorCode::frame_not_ready) return frame"),
            "default/worker helper must not pump; only pending retries may use bounded dispatch and nonzero yield");
    require(owner.find("const auto deadline") < owner.find("for (;;)"),
            "absolute deadline must not restart with each pending frame");
    for (const auto* token : {"windows.h", "PeekMessage", "DispatchMessage", "Sleep(",
                              "thread_local", "->present(", "renderer.resize(", "src/renderer/dx12"})
      require(!has(owner, token), "neutral test wait cannot own Win32, Present, resize, or native fault state");
    require(has(native, "std::min(maximum_messages, 32U)") && has(native, "WM_QUIT") &&
                has(native, "PostQuitMessage(static_cast<int>(message.wParam))"),
            "native leaf must bound message dispatch and preserve surrounding quit requests");
    const auto common_manifest = read("tests/renderer/mcpp.toml");
    const auto dx12_manifest = read("tests/renderer-dx12/mcpp.toml");
    const auto common_build = section(common_manifest, "[build]");
    require(has(common_build, "../support/frame_wait_test_support.cpp") &&
                !has(common_build, "frame_wait_win32_test_support.cpp"),
            "cross-platform renderer math/support library cannot compile native Windows source unconditionally");
    for (const auto& manifest : {common_manifest, dx12_manifest}) {
      const auto windows_build = section(manifest, "[target.'cfg(os = \"windows\")'.build]");
      require(has(windows_build, "../support/frame_wait_win32_test_support.cpp"),
              "native pumping implementation must be explicitly Windows-only in each support library");
    }
    require(has(section(dx12_manifest, "[target.'cfg(os = \"windows\")'.build]"),
                "../support/frame_wait_test_support.cpp") &&
                has(common_manifest, "renderer_frame_wait_contract_test.cpp") &&
                has(read("tests/architecture/mcpp.toml"), "renderer_frame_wait_structure_test.cpp") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "renderer_frame_wait_structure_test|tests/architecture/renderer_frame_wait_structure_test.cpp"),
            "shared implementation, behavior contract and structure guard require explicit discovery");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

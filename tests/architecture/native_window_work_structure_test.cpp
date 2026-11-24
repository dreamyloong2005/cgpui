#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream input(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!input) throw std::runtime_error(std::string("missing native work leaf: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != text.npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
void cap(const char* path, unsigned lines) {
  const auto text = read(path);
  require(std::count(text.begin(), text.end(), '\n') <= lines, "native work module grew beyond its focused leaf");
}
} // namespace

int main() {
  try {
    cap("src/core/native_window_work_internal.hpp", 60);
    cap("src/core/native_window_work_registry_internal.hpp", 60);
    cap("src/core/native_window_work_registry.cpp", 130);
    cap("src/core/native_window_work.cpp", 110);
    cap("src/platform/win32/win32_native_window_work_internal.hpp", 40);
    cap("src/platform/win32/win32_native_window_work.cpp", 80);
    cap("src/platform/win32/win32_window_internal.hpp", 134);
    cap("src/platform/win32/win32_window.cpp", 130);
    cap("src/platform/win32/win32_window_proc.cpp", 80);
    const auto header = read("src/core/native_window_work_internal.hpp");
    require(has(header, "std::weak_ptr<NativeWindowWork>") && has(header, "std::uintptr_t instance") &&
        has(header, "std::uintptr_t work") && !has(header, "windows.h") && !has(header, "std::mutex"),
        "private neutral seam must expose weak work and numeric identities, not native/mutex implementation");
    const auto registry = read("src/core/native_window_work_registry.cpp");
    const auto dispatch = read("src/core/native_window_work.cpp");
    require(has(registry, "found->second->owner != std::this_thread::get_id()") &&
        has(dispatch, "endpoint->second->window != window") &&
        has(dispatch, "endpoint->second->owner != std::this_thread::get_id()") &&
        has(dispatch, "const auto callback = weak.lock()") && has(dispatch, "callback->run()") &&
        has(dispatch, "work.explicit_repost && !work.pending"),
        "owner/instance checks and explicit-only deferred wake retention must remain in the core owner");
    const auto native = read("src/platform/win32/win32_native_window_work.cpp");
    require(has(native, "PostMessageW(") && has(native, "GetWindowThreadProcessId") &&
        has(native, "GetCurrentThreadId()") && has(native, "GetCurrentProcessId()") &&
        !has(native, "SendMessage") && !has(native, "reinterpret_cast") && !has(native, "SetWindowSubclass"),
        "mailbox must enqueue numeric tokens for managed owner-thread dispatch, never callback addresses/subclassing");
    require(has(read("src/platform/win32/win32_native_window_work_internal.hpp"), "WM_APP + 9"),
        "native work message identity changed without protocol review");
    const auto window = read("src/platform/win32/win32_window.cpp");
    require(has(window, "native_work_instance_ = win32_native_window_work_open(hwnd)") &&
        window.find("win32_native_window_work_close(") < window.find("DestroyWindow(hwnd_)") &&
        has(read("src/platform/win32/win32_window_proc.cpp"), "win32_window_proc_handle_native_work("),
        "managed attachment and pre-destruction cancellation must own the endpoint lifetime");
    const auto core_tests = read("tests/core/mcpp.toml");
    for (const auto* test : {"native_window_work_test.cpp", "native_window_work_reentry_test.cpp",
                             "native_window_work_thread_test.cpp"}) require(has(core_tests, test), "missing CPU discovery");
    require(has(read("tests/platform/mcpp.toml"), "win32_native_window_work_test.cpp") &&
        has(read("tests/architecture/mcpp.toml"), "native_window_work_structure_test.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"),
            "native_window_work_structure_test|tests/architecture/native_window_work_structure_test.cpp"),
        "native work behavior/structure registrations must be additive and explicit");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

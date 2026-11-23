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
  if (!input) throw std::runtime_error(std::string("missing pacing module: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void limit(const char* path, std::size_t maximum) {
  const auto text = read(path);
  require(std::count(text.begin(), text.end(), '\n') <= maximum, "pacing owner exceeds focused module budget");
}
} // namespace

int main() {
  try {
    const auto owner = read("src/renderer/dx12/dx12_frame_pacing.cpp");
    const auto header = read("src/renderer/dx12/dx12_frame_pacing_internal.hpp");
    const auto native = read("src/renderer/dx12/dx12_frame_pacing_native.cpp");
    require(has(header, "class Dx12FramePacing") && has(header, "HANDLE handle_") &&
                !has(header, "ComPtr") && !has(header, "Dx12Ptr") && !has(header, "Dx12State"),
            "one private owner holds the app handle/grant without pinning native COM/state");
    require(has(owner, "if (configured_)") && has(owner, "if (granted_)") &&
                has(owner, "std::exchange(handle_, nullptr)") && has(owner, "api_.close(handle)") &&
                has(owner, "Dx12FrameAdmission::disabled") && has(owner, "Dx12FrameAdmission::pending"),
            "explicit reconfiguration, grant, close, disabled and pending policies are required");
    require(has(native, "SetMaximumFrameLatency(latency)") &&
                has(native, "GetFrameLatencyWaitableObject()") &&
                has(native, "WaitForSingleObjectEx(handle, 0, FALSE)") &&
                has(native, "status == WAIT_FAILED ? GetLastError() : ERROR_SUCCESS") &&
                has(native, "CloseHandle(handle)"),
            "native pacing must configure/query, nonblockingly poll, immediately capture errors and close");
    for (const auto* token : {"INFINITE", "Sleep(", "MsgWait", "DispatchMessage", "std::thread",
                             "GetCurrentThreadId", "GetWindowThreadProcessId"})
      require(!has(owner, token) && !has(native, token), "pacing cannot pump, block or impose thread affinity");
    for (const auto* path : {"src/renderer/dx12/dx12_retirement.cpp",
                             "src/renderer/dx12/dx12_synchronization.cpp",
                             "src/renderer/dx12/dx12_geometry_upload_frame.cpp",
                             "src/renderer/dx12/dx12_texture_upload_frame.cpp"}) {
      const auto source = read(path);
      require(!has(source, "Dx12FramePacing") && !has(source, "Dx12FrameAdmission"),
              "admission is not GPU retirement or upload reuse proof");
    }
    for (const auto* path : {"include/cgpui/renderer/renderer_frame.hpp",
                             "include/cgpui/renderer/renderer.hpp"}) {
      const auto source = read(path);
      require(!has(source, "Dx12FramePacing") && !has(source, "GetFrameLatencyWaitableObject"),
              "native pacing ownership must not leak into public renderer APIs");
    }
    limit("src/renderer/dx12/dx12_frame_pacing.cpp", 120);
    limit("src/renderer/dx12/dx12_frame_pacing_internal.hpp", 80);
    limit("src/renderer/dx12/dx12_frame_pacing_native.cpp", 80);
    limit("src/renderer/dx12/dx12_frame_pacing_native_internal.hpp", 80);
    limit("tests/renderer-dx12/dx12_pacing_native_test_support.hpp", 80);
    for (const auto* name : {"dx12_pacing_native_api_test_support.cpp",
                            "dx12_pacing_native_setup_test_support.cpp",
                            "dx12_pacing_native_frame_test_support.cpp",
                            "dx12_pacing_script_test_support.cpp"}) {
      const auto path = std::string("tests/renderer-dx12/") + name;
      limit(path.c_str(), 140);
      require(has(read("tests/renderer-dx12/mcpp.toml"), name), "register focused test support source");
    }
    const auto module = read("tests/renderer-dx12/mcpp.toml");
    const auto windows = module.find("[target.'cfg(os = \"windows\")'.test]");
    require(windows != std::string::npos, "native pacing tests stay Windows-only");
    for (const auto* name : {"dx12_frame_pacing_core_test.cpp", "dx12_frame_pacing_error_test.cpp",
                            "dx12_frame_pacing_native_test.cpp", "dx12_frame_pacing_native_lifetime_test.cpp"})
      require(module.find(name, windows) != std::string::npos, "register every pacing core/native regression");
    require(has(read("tests/architecture/mcpp.toml"), "\"dx12/*_structure_test.cpp\"") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "dx12_frame_pacing_structure_test|tests/architecture/dx12/dx12_frame_pacing_structure_test.cpp"),
            "pacing structure must be registered for local and CI execution");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

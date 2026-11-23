#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error("missing pacing control source: " + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
}

int main() {
  try {
    const auto support = read("tests/renderer-dx12/dx12_pacing_control_test_support.cpp");
    require(has(support, "forwarded.set_maximum_latency(chain, latency)") &&
        has(support, "forwarded.latency_object(chain)") && has(support, "forwarded.close(handle)") &&
        has(support, "dx12_remove_native_device(*info)"),
        "controlled waits must retain actual native configuration, close and removal");
    require(!has(support, "forwarded.poll(") && !has(support, "Sleep(") &&
        !has(support, "ShowWindow(") && !has(support, "create_empty_renderer"),
        "controlled waits must not hide admission semantics behind wait/retry/render substitutes");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_pacing_control_test_support.hpp", "dx12_pacing_control_test_support.cpp",
                            "dx12_frame_pacing_pending_test.cpp", "dx12_frame_pacing_poll_failure_test.cpp",
                            "dx12_frame_pacing_poll_recovery_test.cpp"}) {
      const auto path = std::string("tests/renderer-dx12/") + name;
      const auto source = read(path);
      require(std::count(source.begin(), source.end(), '\n') <= 100, "keep control tests in focused leaves");
      if (!path.ends_with(".hpp")) require(has(manifest, name), "register each controlled native test/source");
    }
    require(has(read("tests/architecture/mcpp.toml"), "\"dx12/*_structure_test.cpp\"") &&
        has(read("scripts/ci/architecture-header-targets.txt"),
            "dx12_pacing_control_structure_test|tests/architecture/dx12/dx12_pacing_control_structure_test.cpp"),
        "register controlled-admission architecture test locally and in CI");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

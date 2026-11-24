#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
namespace {
std::string read(const char* path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error(path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* token) { return source.find(token) != std::string::npos; }
void require(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
}
int main() {
  try {
    const auto test = read("tests/renderer-dx12/dx12_command_reuse_exact_close_failure_test.cpp");
    require(std::count(test.begin(), test.end(), '\n') <= 110, "exact Close control must remain focused");
    for (const auto* forbidden : {"submitted = false", "failed = false", "publish_success(", "parent_reset_succeeded("})
      require(!has(test, forbidden), "exact Close test cannot forge ownership proofs");
    for (const auto* proof : {"dx12_error(E_ACCESSDENIED, \"command list close\")", "rejected.error().message != expected.message",
        "denied_close.allocators != 1", "FAILED(denied_close.allocator_result)", "denied_close.lists != 1",
        "FAILED(denied_close.list_result)", "denied_close.closes != 1", "denied_close.close_result != E_ACCESSDENIED",
        "Dx12CommandEventKind::execute_bundle) != executions + 1", "Dx12CommandEventKind::producer_execute) != submissions",
        "close_events != 1", "dx12_release_native_objects(*state)", "reuse_parent_before_bundle_release(",
        "!bundle_released || !heap_released"})
      require(has(test, proof), "Close target failure and actual unsubmitted-parent retirement proof required");
    require(has(read("tests/renderer-dx12/mcpp.toml"), "dx12_command_reuse_exact_close_failure_test.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"), "dx12_command_reuse_exact_close_structure_test|"),
        "exact Close discovery missing");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

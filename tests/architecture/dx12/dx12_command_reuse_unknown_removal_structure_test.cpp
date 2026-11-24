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
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
}
int main() {
  try {
    const auto test = read("tests/renderer-dx12/dx12_command_reuse_unknown_removal_test.cpp");
    require(std::count(test.begin(), test.end(), '\n') <= 150, "unknown removal control must remain focused");
    for (const auto* fake : {"submitted = false", "failed = false", "parent_reset_succeeded(", "publish_success(",
                             "dx12_wait_idle(", "dx12_release_native_objects(", "removal_reason ="})
      require(!has(test, fake), "unknown removal test must not forge proof or complete before actual removal");
    for (const auto* proof : {"native.signal(q, f, value)", "hidden.signals != 1", "FAILED(hidden.signal_result)",
        "dx12_error(E_ACCESSDENIED, \"fence event registration\")", "rejected.error().message != expected.message",
        "Dx12CommandEventKind::producer_execute", "Dx12CommandEventKind::tail_execute", "unknown_trace(",
        "state->pending_fence_value != hidden.ticket", "dx12_remove_native_device(before)",
        "dx12_recover_removed_device(*state)", "!bundle_released || !heap_released || !tail_released",
        "reuse_parent_before_bundle_release(", "reuse_native_history(", "reuse_trace_pixels_ready("})
      require(has(test, proof), "unknown removal control lost actual submission/failure/removal/retirement evidence");
    require(has(read("tests/renderer-dx12/mcpp.toml"), "dx12_command_reuse_unknown_removal_test.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"), "dx12_command_reuse_unknown_removal_structure_test|"),
        "unknown removal control discovery must stay additive");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

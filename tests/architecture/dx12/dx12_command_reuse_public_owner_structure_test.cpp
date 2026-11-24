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
    const auto test = read("tests/renderer-dx12/dx12_command_reuse_public_owner_test.cpp");
    const auto support = read("tests/renderer-dx12/dx12_command_reuse_public_owner_test_support.cpp");
    const auto header = read("tests/renderer-dx12/dx12_command_reuse_public_owner_test_support.hpp");
    require(std::count(test.begin(), test.end(), '\n') <= 120 && std::count(support.begin(), support.end(), '\n') <= 60 &&
        std::count(header.begin(), header.end(), '\n') <= 15, "public owner control must stay in focused test/support leaves");
    for (const auto* source : {&test, &support})
      for (const auto* fake : {"owner_alive = false", "submitted = false", "failed = false", "dx12_release_native_objects(",
                              "reuse_raw_submit(", "srv_native_state(", "std::make_unique<Dx12Frame>", "dx12_register_device_cohort("})
        require(!has(*source, fake), "public owner control cannot substitute private lifecycle/submit/registration");
    for (const auto* proof : {"create_dx12_renderer(window.descriptor())", "wait_for_test_frame(*renderer)", "renderer.reset();",
        "weak.use_count() != 1", "orphan->present()", "orphan.reset();", "!weak.expired()", "!parent_released || !bundle_released || !heap_released",
        "reuse_native_history(", "reuse_trace_pixels_ready(", "info.device_generation", "ErrorCode::invalid_argument"})
      require(has(test, proof), "public BUNDLE orphan control lost actual facade/frame/epoch/pixel/retirement evidence");
    require(has(support, "std::weak_ptr<cgpui::Dx12State> reuse_public_state") && has(support, "dx12_device_cohort_peers(lookup)") &&
        has(support, "state->hwnd == hwnd"), "fixture observation must identify real public state and return weak only");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    require(has(manifest, "dx12_command_reuse_public_owner_test.cpp") && has(manifest, "dx12_command_reuse_public_owner_test_support.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"), "dx12_command_reuse_public_owner_structure_test|"), "public owner discovery missing");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

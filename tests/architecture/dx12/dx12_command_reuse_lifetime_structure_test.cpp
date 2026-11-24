#include <algorithm>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::filesystem::path& path) {
  std::ifstream file(root / path);
  if (!file) throw std::runtime_error("missing command-reuse lifetime leaf: " + path.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& s, const char* token) { return s.find(token) != std::string::npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
void pure_leaf(const char* name, std::size_t limit) {
  const auto s = read(std::string("src/renderer/dx12/") + name);
  require(std::count(s.begin(), s.end(), '\n') <= limit, "reuse proof/epoch leaf exceeded focused budget");
  for (const auto* forbidden : {"#include <windows", "#include <d3d12", "vulkan", "dx12_state_internal.hpp",
      "ExecuteCommandLists(", "ExecuteBundle(", "CreateCommittedResource(", "DrawInstanced(",
      "DrawIndexedInstanced(", "Reset(", "std::vector", "std::shared_ptr", "std::unique_ptr"})
    require(!has(s, forbidden), "pure proof leaves cannot own native operations/resources or dynamic containers");
}
}
int main() {
  try {
    pure_leaf("dx12_command_reuse_lifetime_internal.hpp", 100);
    pure_leaf("dx12_command_reuse_lifetime.cpp", 180);
    pure_leaf("dx12_command_reuse_surface_epoch_internal.hpp", 60);
    pure_leaf("dx12_command_reuse_surface_epoch.cpp", 60);
    const auto header = read("src/renderer/dx12/dx12_command_reuse_lifetime_internal.hpp");
    const auto public_start = header.find("public:");
    const auto private_start = header.find("private:");
    require(public_start < private_start && private_start != std::string::npos, "private proof boundary missing");
    const auto visible = header.substr(public_start, private_start - public_start);
    for (const auto* name : {"begin_generation(", "create_entry(", "attach_parent(", "reserve_submission(",
        "completed(", "removed(", "parent_reset_succeeded(", "parent_released(", "publish_success(",
        "invalidate(", "forget_released_entry(", "return "})
      require(!has(visible, name), "proof mutation/implementation must not become public convenience API");
    require(has(header, "friend class Dx12FrameCommandReuse;") &&
        has(header, "friend class Dx12CommandReuseTestAccess;") &&
        !has(header, "Dx12ParentCommands") && !has(header, "Dx12CommandReuseRetirement"),
        "only the implemented native owner and test access may hold proof authority");
    const auto source = read("src/renderer/dx12/dx12_command_reuse_lifetime.cpp");
    require(has(source, "gpu_retired() && parent_detached()") &&
        has(source, "gpu_ = GpuUse::pending") && has(source, "submission != submission_") &&
        has(source, "parent <= last_parent_") && has(source, "submission <= last_submission_"),
        "independent G/P and stale-identity guards must remain visible");
    const auto surface = read("src/renderer/dx12/dx12_command_reuse_surface_epoch.cpp");
    require(has(surface, "key.scale == key_.scale") && has(surface, "std::isfinite(key.scale)") &&
        has(surface, "exhausted_ = true") && !has(surface, "Dx12CommandReuseLifetime"),
        "surface identity must track scale without acquiring resource-release authority");
    for (const auto* name : {"dx12_renderer.cpp", "dx12_state.cpp", "dx12_frame.cpp", "dx12_presentation.cpp"}) {
      const auto s = read(std::string("src/renderer/dx12/") + name);
      require(!has(s, "Dx12CommandReuseLifetime::") && !has(s, "Dx12CommandReuseSurfaceEpoch::"),
          "orchestration cannot absorb private lifecycle definitions");
    }
    for (const auto& item : std::filesystem::recursive_directory_iterator(root / "include/cgpui")) {
      if (!item.is_regular_file() || item.path().extension() != ".hpp") continue;
      const auto s = read(item.path());
      require(!has(s, "Dx12CommandReuseLifetime") && !has(s, "Dx12CommandReuseSurfaceEpoch") &&
          !has(s, "Dx12CommandReuseTestAccess"), "reuse proofs/test access must remain private");
    }
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_command_reuse_test_support.cpp", "dx12_command_reuse_lifetime_test.cpp",
        "dx12_command_reuse_proof_test.cpp", "dx12_command_reuse_surface_epoch_test.cpp"})
      require(has(manifest, name), "mcpp must discover focused support and CPU tests");
    require(has(read("packages/renderer-dx12/mcpp.toml"), "../../src/renderer/dx12/*.cpp") &&
        has(read("tests/architecture/mcpp.toml"), "dx12/*_structure_test.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"), "dx12_command_reuse_lifetime_structure_test|"),
        "mcpp and CI must retain module/test discovery");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

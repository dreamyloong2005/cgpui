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
  if (!input) throw std::runtime_error(std::string("missing pacing creation owner: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& value, const char* token) { return value.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void limit(const char* path, std::size_t maximum) {
  const auto text = read(path);
  require(std::count(text.begin(), text.end(), '\n') <= maximum, "pacing integration exceeded existing/focused cap");
}
} // namespace

int main() {
  try {
    const auto renderer = read("src/renderer/dx12/dx12_renderer.cpp");
    require(has(renderer, "dx12_create_renderer(descriptor, dx12_native_frame_pacing_api())") &&
                has(renderer, "std::make_shared<Dx12State>(&pacing_api, synchronization_api)") &&
                has(renderer, "std::make_unique<Dx12Renderer>(std::move(state))"),
            "public native-default and private API seam must use the same actual renderer creation path");
    const auto state = read("src/renderer/dx12/dx12_state_internal.hpp");
    require(has(state, "std::unique_ptr<Dx12FramePacing> frame_pacing") &&
                !has(state, "HANDLE pacing") && !has(state, "bool pacing"),
            "private state delegates unique handle/grant ownership to its focused module");
    const auto constructor = read("src/renderer/dx12/dx12_state.cpp");
    require(has(constructor, "pacing_api ? *pacing_api : dx12_native_frame_pacing_api()") &&
                has(constructor, "synchronization_api(synchronization)"),
            "state retains an outliving private API without a production fake-switch");
    const auto swapchain = read("src/renderer/dx12/dx12_swapchain.cpp");
    const auto resize = read("src/renderer/dx12/dx12_swapchain_resize.cpp");
    const auto flags = read("src/renderer/dx12/dx12_swapchain_flags_internal.hpp");
    require(has(swapchain, "state.frame_pacing->configure(state.swapchain.Get(), waitable)") &&
                has(swapchain, "(descriptor.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT) != 0") &&
                !has(swapchain, "SetMaximumFrameLatency(") &&
                !has(swapchain, "Result<void> dx12_resize(") && !has(resize, "frame_pacing") &&
                !has(resize, "GetFrameLatencyWaitableObject") && !has(resize, "SetMaximumFrameLatency"),
            "one acquisition owner configures both new-chain routes and leaves resize ownership alone");
    require(has(flags, "constexpr UINT dx12_swapchain_flags() noexcept") &&
                has(flags, "return DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;") &&
                has(swapchain, "descriptor.Flags = dx12_swapchain_flags();") &&
                has(resize, "dx12_swapchain_flags());") &&
                !has(swapchain, "state.composition_surface ? 0") && !has(flags, "composition_surface"),
            "creation and ResizeBuffers must share the unchanged waitable flag for both native routes");
    const auto release = read("src/renderer/dx12/dx12_native_retirement.cpp");
    const auto reset = release.find("state.frame_pacing->reset()");
    require(reset != std::string::npos && reset < release.find("state.swapchain.Reset()"),
            "native retirement must close admission before releasing its old chain");
    const auto private_header = read("src/renderer/dx12/dx12_renderer_creation_internal.hpp");
    require(has(private_header, "must outlive the renderer AND every outstanding frame") &&
                !has(read("include/cgpui/renderer/renderer_backend.hpp"), "Dx12FramePacingApi"),
            "borrowed seam lifetime is explicit and is not an application API");
    limit("src/renderer/dx12/dx12_renderer.cpp", 100);
    limit("src/renderer/dx12/dx12_swapchain_flags_internal.hpp", 20);
    limit("src/renderer/dx12/dx12_swapchain.cpp", 70);
    limit("src/renderer/dx12/dx12_swapchain_resize.cpp", 40);
    limit("src/renderer/dx12/dx12_renderer_creation_internal.hpp", 30);
    limit("tests/renderer-dx12/dx12_pacing_begin_test_support.hpp", 30);
    limit("tests/renderer-dx12/dx12_pacing_begin_wait_test_support.cpp", 80);
    limit("tests/renderer-dx12/dx12_pacing_begin_case_test_support.cpp", 90);
    const auto module = read("tests/renderer-dx12/mcpp.toml");
    const auto windows = module.find("[target.'cfg(os = \"windows\")'.test]");
    require(windows != std::string::npos, "real first-begin probes are Windows-only");
    for (const auto* name : {"dx12_frame_pacing_begin_hidden_test.cpp", "dx12_frame_pacing_begin_visible_test.cpp",
                            "dx12_frame_pacing_creation_lifetime_test.cpp"})
      require(module.find(name, windows) != std::string::npos, "register first-begin/lifetime acceptance");
    require(has(module, "\"dx12_pacing_begin_wait_test_support.cpp\"") &&
                has(module, "\"dx12_pacing_begin_case_test_support.cpp\"") &&
                has(read("tests/architecture/mcpp.toml"), "\"dx12/*_structure_test.cpp\"") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "dx12_frame_pacing_creation_structure_test|tests/architecture/dx12/dx12_frame_pacing_creation_structure_test.cpp"),
            "register focused implementations and structure validation without replacing other entries");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

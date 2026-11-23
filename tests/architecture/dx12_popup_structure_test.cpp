#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* name) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / name);
  if (!stream) throw std::runtime_error(std::string("missing popup owner: ") + name);
  return {std::istreambuf_iterator<char>(stream), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
} // namespace

int main() {
  try {
    for (const auto* path : {"src/renderer/dx12/dx12_composition_internal.hpp",
                            "src/renderer/dx12/dx12_composition.cpp",
                            "src/renderer/dx12/dx12_surface_internal.hpp",
                            "src/renderer/dx12/dx12_surface.cpp"}) {
      const auto text = read(path);
      require(std::count(text.begin(), text.end(), '\n') <= 150,
              "composition hosting and surface policy need focused private owners");
      for (const auto* banned : {"D3D11CreateDevice", "IDXGIKeyedMutex", "UpdateLayeredWindow",
                                 "Win32CompositionPresenter", "../vulkan", "getenv("})
        require(!has(text, banned), "direct DX12 Popup must not use interop/copy or public mode switches");
    }
    const auto swapchain = read("src/renderer/dx12/dx12_swapchain.cpp");
    require(has(swapchain, "CreateSwapChainForComposition(") && has(swapchain, "CreateSwapChainForHwnd(") &&
                has(swapchain, "DXGI_ALPHA_MODE_PREMULTIPLIED") && has(swapchain, "DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL"),
            "transparent Popup and ordinary HWND must retain distinct private presentation paths");
    const auto surface = read("src/renderer/dx12/dx12_surface.cpp");
    require(has(surface, "WS_EX_NOREDIRECTIONBITMAP") && has(surface, "dx12_surface_clear(") &&
                has(surface, "RendererCompositeAlpha::pre_multiplied"),
            "surface policy owns host validation, actual alpha reporting and premultiplied clear");
    const auto renderer = read("src/renderer/dx12/dx12_renderer.cpp");
    require(has(renderer, "dx12_configure_surface(") && has(renderer, "dx12_surface_composition(") &&
                !has(renderer, "DCompositionCreate") && !has(renderer, "CreateSwapChainForComposition"),
            "renderer entry must delegate native hosting and surface intent");
    const auto submit = read("src/renderer/dx12/dx12_presentation.cpp");
    require(has(submit, "dx12_surface_clear(") && has(submit, "dx12_commit_composition(") &&
                submit.find("dx12_commit_composition(") > submit.find("FAILED(presented)"),
            "first composition commit follows successful native presentation");
    const auto retirement = read("src/renderer/dx12/dx12_native_retirement.cpp");
    require(has(retirement, "dx12_release_composition(") &&
                retirement.find("dx12_release_composition(") < retirement.find("state.swapchain.Reset()"),
            "native retirement must release visual ownership before its swapchain/device");
    const auto package = read("packages/renderer-dx12/mcpp.toml");
    require(has(package, "dcomp.lib") && !has(package, "d3d11.lib") &&
                !has(package, "renderer-win32-composition"),
            "DX12 Popup must not pull the D3D11 sharing bridge into its package");
    const auto shader = read("src/renderer/vulkan/shaders/backdrop.slang");
    require(has(shader, "composite_alpha_fragment(") && has(shader, "scene_snapshot.Load(") &&
                has(shader, "lerp(scene, tinted, mask * input.region.w)"),
            "transparent backdrop must replace premultiplied RGBA using the original scene");
    const auto composite = read("src/renderer/dx12/dx12_backdrop_blur_composite.cpp");
    require(has(composite, "state.composition_surface") && has(composite, "alpha_composite_pipeline") &&
                has(composite, "SetGraphicsRootDescriptorTable(2, targets.targets[0].srv)") &&
                has(retirement, "alpha_composite_pipeline.Reset()"),
            "alpha replacement owns the original-scene binding and follows native retirement");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

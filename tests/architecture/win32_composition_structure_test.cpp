#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::string read(const char* path) {
  const auto root = CGPUI_SOURCE_ROOT;
  std::ifstream stream(std::filesystem::path(root ? root : ".") / path);
  if (!stream) throw std::runtime_error(std::string("missing focused module: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
bool has(const std::string& source, std::string_view text) {
  return source.find(text) != std::string::npos;
}
std::size_t lines(const std::string& source) {
  return std::count(source.begin(), source.end(), '\n') + 1;
}
// Vulkan headers, types, enums or calls (comments may name Vulkan).
bool vulkan_code(const std::string& source) {
  for (std::size_t at = source.find("vk"); at != std::string::npos; at = source.find("vk", at + 1)) {
    if (at + 2 < source.size() && source[at + 2] >= 'A' && source[at + 2] <= 'Z') return true;
  }
  return has(source, "<vulkan/") || has(source, "Vk") || has(source, "VK_");
}
} // namespace

// Popup windows on Win32 present through DirectComposition in two parts
// (docs/superpowers/plans/2026-11-22-popup-windows-windows-linux-follow-up.md,
// "presentation split for a DX12 backend"): a composition module without
// GPU backend code, and the Vulkan side that imports its texture.
int main() {
  try {
    const auto package = read("packages/renderer-win32-composition/mcpp.toml");
    const auto workspace = read("mcpp.toml");
    require(has(workspace, "\"packages/renderer-win32-composition\"") &&
                has(package, "cgpui.core") && !has(package, "cgpui.renderer") &&
                !has(package, "cgpui.platform") &&
                has(package, "\"../../src/renderer/win32_composition/*.cpp\"") &&
                has(package, "\"d3d11.lib\", \"dxgi.lib\", \"dcomp.lib\"") &&
                has(package, "empty_library.cpp"),
            "the composition module is a Windows-only package over core only");
    struct Source {
      const char* path;
      std::size_t max_lines;
    };
    constexpr Source module_sources[]{
        {"src/renderer/win32_composition/win32_composition_presenter_internal.hpp", 80},
        {"src/renderer/win32_composition/win32_composition_objects_internal.hpp", 60},
        {"src/renderer/win32_composition/win32_composition_device.cpp", 130},
        {"src/renderer/win32_composition/win32_composition_texture.cpp", 60},
        {"src/renderer/win32_composition/win32_composition_presenter.cpp", 120},
    };
    for (const Source& source : module_sources) {
      const auto text = read(source.path);
      require(lines(text) <= source.max_lines, source.path);
      require(!vulkan_code(text),
              "the composition module holds no Vulkan code (a DX12 backend reuses it)");
    }
    const auto api = read("src/renderer/win32_composition/win32_composition_presenter_internal.hpp");
    const auto device = read("src/renderer/win32_composition/win32_composition_device.cpp");
    const auto texture = read("src/renderer/win32_composition/win32_composition_texture.cpp");
    const auto presenter = read("src/renderer/win32_composition/win32_composition_presenter.cpp");
    require(has(api, "LUID adapter, HWND window, Win32CompositionSize size)") &&
                has(api, "HANDLE shared_texture_handle() const") &&
                has(api, "bool win32_composition_available()"),
            "the module takes a LUID, an HWND and a size and exposes the texture handle");
    require(has(device, "EnumAdapterByLuid") && has(device, "CreateSwapChainForComposition") &&
                has(device, "DXGI_ALPHA_MODE_PREMULTIPLIED") &&
                has(device, "CreateTargetForHwnd") && has(device, "DCompositionCreateDevice"),
            "the module owns the LUID device, the composition swapchain and DComp");
    require(has(texture, "D3D11_RESOURCE_MISC_SHARED_NTHANDLE") &&
                has(texture, "D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX") &&
                has(texture, "CreateSharedHandle") &&
                has(presenter, "AcquireSync(") &&
                has(presenter, "win32_composition_present_key,") &&
                has(presenter, "ReleaseSync(win32_composition_render_key)") &&
                has(presenter, "ResizeBuffers"),
            "the module owns the shared texture and the D3D11 acquire, copy, release and present");
    const auto tests = read("tests/renderer/mcpp.toml");
    const auto behavior = read("tests/renderer/win32_composition_presenter_test.cpp");
    const auto windows_tests = tests.find("[target.'cfg(os = \"windows\")'.test]");
    require(windows_tests != std::string::npos &&
                tests.find("\"win32_composition_presenter_test.cpp\"") > windows_tests &&
                !vulkan_code(behavior) && !has(behavior, "create_renderer("),
            "a Windows test presents through the module with D3D11 alone");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream source(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!source) throw std::runtime_error(std::string("missing backend owner: ") + path);
  return {std::istreambuf_iterator<char>(source), {}};
}
bool has(const std::string& text, const char* token) {
  return text.find(token) != std::string::npos;
}
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
}

int main() {
  try {
    for (const auto* path : {"include/cgpui/renderer/renderer_backend.hpp",
                             "include/cgpui/renderer/renderer_backend_info.hpp"}) {
      const auto text = read(path);
      require(std::count(text.begin(), text.end(), '\n') <= 45 &&
                  !has(text, "windows.h") && !has(text, "vulkan.h") &&
                  !has(text, "inline ") && !has(text, "getenv"),
              "backend public leaves must remain small and platform-neutral");
    }
    const auto factory = read("src/renderer/windows/windows_renderer_factory.cpp");
    require(has(factory, "select_renderer_backend(") && has(factory, "CGPUI_RENDERER") &&
                has(factory, "create_vulkan_renderer") && has(factory, "create_dx12_renderer") &&
                std::count(factory.begin(), factory.end(), '\n') <= 50,
            "Windows dispatcher only reads environment and delegates selection");
    const auto dispatcher = read("packages/renderer-windows/mcpp.toml");
    require(has(dispatcher, "renderer-vulkan") && has(dispatcher, "renderer-dx12") &&
                has(dispatcher, "src/renderer/windows/*.cpp"),
            "dispatcher package must own both native backend dependencies");
    for (const auto* package : {"packages/renderer/mcpp.toml",
                                "packages/renderer-vulkan/mcpp.toml",
                                "packages/renderer-dx12/mcpp.toml"}) {
      require(!has(read(package), "renderer-windows"), "native/neutral packages cannot depend upward");
    }
    const auto selection = read("src/renderer/renderer_backend_selection.cpp");
    require(!has(selection, "create_dx12_renderer") && !has(selection, "create_vulkan_renderer") &&
                !has(selection, "getenv") && !has(selection, "#include <vulkan") &&
                std::count(selection.begin(), selection.end(), '\n') <= 65,
            "selection policy must remain GPU-free and injectable");
    const auto linux_factory = read("src/renderer/vulkan/vulkan_default_factory_linux.cpp");
    require(has(linux_factory, "defined(__linux__)") && has(linux_factory, "create_vulkan_renderer") &&
                !has(linux_factory, "dx12") && !has(linux_factory, "getenv"),
            "Linux factory remains an explicit Vulkan forwarder");
    const auto native = read("src/renderer/vulkan/vulkan_renderer.cpp");
    require(has(native, "create_vulkan_renderer(") && !has(native, "create_renderer("),
            "Vulkan must not define the Windows dispatcher symbol");
    require(has(read("src/renderer/vulkan/vulkan_state_internal.hpp"),
                "#include \"vulkan_frame_effects_internal.hpp\""),
            "Vulkan state must declare its effect return type independently of umbrella order");
    const auto info = read("src/ui/window_backend_info.cpp");
    require(has(info, "renderer->backend_info()") &&
                std::count(info.begin(), info.end(), '\n') <= 25,
            "Window backend information has a focused forwarding owner");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

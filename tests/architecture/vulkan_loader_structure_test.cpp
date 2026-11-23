#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::filesystem::path& path) {
  std::ifstream input(root / path);
  if (!input) throw std::runtime_error("missing Vulkan loader owner: " + path.generic_string());
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& source, const char* token) {
  return source.find(token) != std::string::npos;
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
void cap(const char* path, unsigned maximum) {
  const auto source = read(path);
  if (std::count(source.begin(), source.end(), '\n') > maximum)
    throw std::runtime_error(std::string("focused module cap exceeded: ") + path);
}
} // namespace

int main() {
  try {
    cap("src/renderer/vulkan/vulkan_loader_windows_internal.hpp", 80);
    cap("src/renderer/vulkan/vulkan_loader_windows.cpp", 100);
    cap("src/renderer/vulkan/vulkan_loader_windows_symbols.cpp", 70);
    cap("src/renderer/vulkan/vulkan_loader_windows_symbols.inc", 110);
    // Keep the pre-existing entry-point allowances; loading is a private leaf.
    cap("src/renderer/vulkan/vulkan_internal.hpp", 120);
    cap("src/renderer/vulkan/vulkan_renderer.cpp", 240);
    cap("src/renderer/vulkan/vulkan_state.cpp", 170);
    cap("src/renderer/vulkan/vulkan_instance_surface.cpp", 120);

    const auto header = read("src/renderer/vulkan/vulkan_loader_windows_internal.hpp");
    const auto loader = read("src/renderer/vulkan/vulkan_loader_windows.cpp");
    const auto resolver = read("src/renderer/vulkan/vulkan_loader_windows_symbols.cpp");
    const auto symbols = read("src/renderer/vulkan/vulkan_loader_windows_symbols.inc");
    require(has(header, "namespace cgpui") && has(header, "defined(_WIN32)") &&
                has(header, "struct VulkanWindowsFunctions") &&
                has(header, "VulkanWindowsResolver") &&
                has(header, "ensure_vulkan_windows_loader()") &&
                has(header, "resolve_vulkan_windows_functions(") &&
                has(header, "extern PFN_") &&
                has(header, "vulkan_loader_windows_symbols.inc"),
            "typed Windows dispatch declarations must remain in the private loader leaf");
    for (const auto* source : {&loader, &resolver}) {
      require(has(*source, "vulkan_loader_windows_internal.hpp") &&
                  has(*source, "defined(_WIN32)"),
              "loader implementation is private and Windows-only");
    }
    require(has(loader, "LoadLibraryExW(") &&
                has(loader, "LOAD_LIBRARY_SEARCH_DEFAULT_DIRS") &&
                has(loader, "vulkan-1.dll") && has(loader, "GetLastError()") &&
                has(loader, "ErrorCode::renderer_initialization_failed") &&
                has(loader, "std::mutex") && has(loader, "std::lock_guard") &&
                has(loader, "resolve_vulkan_windows_functions(") &&
                has(loader, "decltype(&FreeLibrary)") && has(loader, ".release()"),
            "private loader owns safe DLL search, typed failure, serialization and lifetime");
    require(has(loader, "if (!candidate) return std::unexpected(candidate.error())") &&
                has(loader, "name = candidate->name") && has(loader, "ready = true") &&
                loader.find("resolve_vulkan_windows_functions(") < loader.find("name = candidate->name") &&
                loader.find("name = candidate->name") < loader.find("module.release()") &&
                loader.find("module.release()") < loader.find("ready = true"),
            "only a complete candidate may publish; successful code stays pinned before readiness");
    for (const auto* banned : {"LoadLibraryW(", "LoadLibraryA(", "SetDllDirectory",
                              "SetDefaultDllDirectories", "std::call_once", "atexit("}) {
      require(!has(loader, banned),
              "loading must not alter process search policy or cache failure permanently");
    }
    require(has(resolver, "resolve_vulkan_windows_functions(") &&
                has(resolver, "VulkanWindowsFunctions") &&
                has(resolver, "vulkan_loader_windows_symbols.inc") &&
                has(resolver, "std::unexpected") && has(resolver, "PFN_") &&
                has(resolver, "resolver(module, #name)") &&
                !has(resolver, "name = candidate->name"),
            "symbol resolution belongs to a focused typed candidate-table owner");
    for (const auto* name : {"vkGetInstanceProcAddr", "vkCreateInstance", "vkDestroyInstance",
                            "vkCreateWin32SurfaceKHR", "vkCreateDevice", "vkDestroyDevice",
                            "vkQueueSubmit", "vkQueuePresentKHR"}) {
      require(has(symbols, name), "shared symbol inventory must cover bootstrap and rendering");
    }
    require(!has(symbols, "LoadLibrary") && !has(symbols, "GetProcAddress") &&
                !has(symbols, "#include"),
            "symbol inventory must not grow executable loader policy");

    const auto instance = read("src/renderer/vulkan/vulkan_instance_surface.cpp");
    const auto platform = read("src/renderer/vulkan/vulkan_platform_internal.hpp");
    require(has(platform, "vulkan_loader_windows_internal.hpp") &&
                !has(platform, "VK_NO_PROTOTYPES"),
            "private platform aggregate exposes typed names without changing global Vulkan prototypes");
    require(has(instance, "ensure_vulkan_windows_loader()") &&
                has(instance, "vkGetInstanceProcAddr(") &&
                instance.find("ensure_vulkan_windows_loader()") <
                    instance.find("vkGetInstanceProcAddr("),
            "instance creation must gate the first Vulkan call on loader availability");
    for (const auto* path : {"src/renderer/vulkan/vulkan_instance_surface.cpp",
                             "src/renderer/vulkan/vulkan_renderer.cpp",
                             "src/renderer/vulkan/vulkan_state.cpp",
                             "src/renderer/vulkan/vulkan_internal.hpp"}) {
      const auto source = read(path);
      require(!has(source, "LoadLibrary") && !has(source, "GetProcAddress(") &&
                  !has(source, "VulkanWindowsFunctions") && !has(source, "std::mutex"),
              "renderer and instance entries must delegate loading rather than own it");
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root / "include/cgpui/renderer")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".hpp") continue;
      const auto source = read(entry.path());
      for (const auto* banned : {"<windows.h>", "<vulkan/", "PFN_vk", "HMODULE",
                                "vulkan_loader_windows", "LoadLibrary", "GetProcAddress"}) {
        require(!has(source, banned), "public renderer headers must not expose native loader details");
      }
    }

    const auto build = read("packages/renderer-vulkan/build.mcpp");
    require(has(build, "target_is(\"windows\")") && has(build, "vulkan_sdk(false)") &&
                !has(build, "vulkan_sdk(true)") &&
                has(build, "target_is(\"linux\")") &&
                has(build, "pkg_config({\"vulkan\", \"xcb\", \"wayland-client\"})") &&
                build.find("vulkan_sdk(false)") < build.find("target_is(\"linux\")"),
            "Windows needs Vulkan headers only; Linux must retain native pkg-config linking");
    require(has(read("packages/renderer-vulkan/mcpp.toml"), "../../src/renderer/vulkan/*.cpp"),
            "loader implementation must be built by the owning Vulkan package");
    for (const auto* path : {"packages/renderer-vulkan/build.mcpp",
                             "packages/renderer-vulkan/mcpp.toml",
                             "packages/renderer-windows/mcpp.toml",
                             "scripts/ci/sdk-package.py", "scripts/ci/sdk_package_cmake.py"}) {
      const auto source = read(path);
      for (const auto* banned : {"DELAYLOAD", "delayload", "delayimp", "__pfnDli", "__delayLoadHelper"})
        require(!has(source, banned), "SDK consumers must not require delay-load flags, libraries or hooks");
    }
    require(has(read("tests/renderer-dx12/mcpp.toml"), "dx12_loader_independence_test.cpp"),
            "native DX12 loader-independence regression must stay registered");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Vulkan loader structure: " << error.what() << '\n';
    return 1;
  }
}

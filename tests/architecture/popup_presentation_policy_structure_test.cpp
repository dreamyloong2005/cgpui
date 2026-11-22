#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
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
void limit(const char* path, std::size_t max_lines) {
  const auto source = read(path);
  const auto count = static_cast<std::size_t>(std::count(source.begin(), source.end(), '\n')) +
                     (!source.empty() && source.back() != '\n' ? 1U : 0U);
  require(count <= max_lines,
          path);
}
} // namespace

// The application requests popup behavior, not a Windows presentation path.
// Backend support and runtime negotiation each own a focused implementation.
int main() {
  try {
    limit("src/renderer/renderer_popup_support.cpp", 30);
    limit("src/renderer/vulkan/vulkan_popup_support_internal.hpp", 25);
    limit("src/renderer/vulkan/vulkan_popup_support.cpp", 40);
    limit("src/ui/runtime_popup_support_internal.hpp", 30);
    limit("src/ui/runtime_popup_support.cpp", 60);
    limit("src/ui/runtime_popups.cpp", 120);
    limit("src/ui/runtime_window_activation.cpp", 70);
    limit("src/renderer/vulkan/vulkan_renderer.cpp", 240);
    limit("src/renderer/metal/metal_device.mm", 120);

    const auto renderer_api = read("include/cgpui/renderer/renderer_frame.hpp");
    const auto renderer_default = read("src/renderer/renderer_popup_support.cpp");
    require(std::regex_search(renderer_api, std::regex(
                R"(virtual\s+bool\s+popup_windows_supported\s*\(\s*\)\s*const\s*;)")) &&
                has(renderer_default, "Renderer::popup_windows_supported") &&
                has(renderer_default, "return false"),
            "renderer popup support is declared at the leaf interface and defaults "
            "to unsupported in its own source");
    for (const auto* path : {"include/cgpui/renderer/renderer.hpp",
                            "include/cgpui/platform/platform.hpp",
                            "include/cgpui/cgpui.hpp"}) {
      const auto aggregate = read(path);
      require(!has(aggregate, "popup_windows_supported(") &&
                  !has(aggregate, "DCompositionCreate"),
              "aggregate headers must not implement popup policy or presentation");
    }

    const auto vulkan_header =
        read("src/renderer/vulkan/vulkan_popup_support_internal.hpp");
    const auto vulkan_policy = read("src/renderer/vulkan/vulkan_popup_support.cpp");
    const auto vulkan_renderer = read("src/renderer/vulkan/vulkan_renderer.cpp");
    require(has(vulkan_header, "vulkan_popup_windows_supported") &&
                has(vulkan_policy, "vulkan_popup_windows_supported") &&
                !has(vulkan_policy, "_WIN32") &&
                has(vulkan_renderer, "vulkan_popup_support_internal.hpp") &&
                has(vulkan_renderer, "vulkan_popup_windows_supported("),
            "Vulkan declares popup capability in its private module without a Windows blanket ban");
    require(!has(vulkan_header, "windows.h") && !has(vulkan_header, "dcomp.h") &&
                !has(vulkan_header, "return ") &&
                has(read("src/renderer/metal/metal_device.mm"), "popup_windows_supported"),
            "policy declarations stay thin and Metal keeps its own capability");

    const auto runtime_policy = read("src/ui/runtime_popup_support.cpp");
    const auto popups = read("src/ui/runtime_popups.cpp");
    const auto activation = read("src/ui/runtime_window_activation.cpp");
    require(has(runtime_policy, "popup_windows_supported") &&
                has(runtime_policy, "RuntimePopups::supported") &&
                !has(popups, "bool RuntimePopups::supported"),
            "runtime popup negotiation has one focused owner");
    require(!has(runtime_policy, "Win32") && !has(runtime_policy, "Vulkan") &&
                !has(runtime_policy, "DirectComposition") &&
                !has(runtime_policy, "_WIN32"),
            "runtime policy consumes capabilities rather than selecting platform mechanisms");
    require(has(runtime_policy, "runtime_accept_popup_renderer") &&
                has(runtime_policy, "WindowKind::popup") &&
                has(runtime_policy, "unsupported_platform") &&
                has(activation, "runtime_popup_support_internal.hpp") &&
                !has(activation, "WindowKind::popup") &&
                !has(activation, "unsupported_platform"),
            "popup refusal and diagnostics belong in the policy module, not window activation");
    const auto parent_guard = activation.find("runtime_accept_popup_renderer(");
    const auto child_guard = activation.rfind("runtime_accept_popup_renderer(");
    const auto create_window = activation.find("create_child_window(");
    const auto create_renderer = activation.find("try_create_renderer(");
    const auto activate = activation.find("record.active = true");
    require(activate != std::string::npos && parent_guard < create_window &&
                create_window < create_renderer && create_renderer < child_guard &&
                child_guard < activate,
            "activation must delegate before native creation and again before activating a child");

    const std::regex presentation_member(R"(\bpresentation\s*(?:\(|=|;|\{))");
    for (const auto* path : {"include/cgpui/core/window.hpp",
                            "include/cgpui/ui/runtime_window_options.hpp"}) {
      const auto options = read(path);
      require(!has(options, "WindowPresentation") &&
                  !has(options, "window_presentation.hpp") &&
                  !std::regex_search(options, presentation_member) &&
                  !has(options, "WS_EX_NOREDIRECTIONBITMAP"),
              "application window options must not expose native/composition presentation");
    }
    require(has(read("tests/architecture/mcpp.toml"),
                "popup_presentation_policy_structure_test.cpp") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "popup_presentation_policy_structure_test|tests/architecture/"
                    "popup_presentation_policy_structure_test.cpp") &&
                has(read("tests/ui/mcpp.toml"), "window_runtime_popup_capability_test.cpp") &&
                has(read("tests/ui/mcpp.toml"), "workbench_popup_fallback_test.cpp"),
            "capability and overlay behavior tests must accompany the structure guard");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}

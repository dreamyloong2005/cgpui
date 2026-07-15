#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  std::ifstream source(path);
  if (!source) {
    source.open((std::string("../../../../") + path).c_str());
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

} // namespace

int main() {
  if (cgpui::default_renderer_backend_for(cgpui::DesktopPlatformTarget::windows) !=
      cgpui::RendererBackendTarget::vulkan) {
    return 1;
  }
  if (cgpui::default_renderer_backend_for(cgpui::DesktopPlatformTarget::linux_wayland) !=
      cgpui::RendererBackendTarget::vulkan) {
    return 2;
  }
  if (cgpui::default_renderer_backend_for(cgpui::DesktopPlatformTarget::macos_cocoa) !=
      cgpui::RendererBackendTarget::metal) {
    return 3;
  }
  if (cgpui::desktop_platform_target_name(cgpui::DesktopPlatformTarget::macos_cocoa) !=
      std::string_view{"macOS/Cocoa"}) {
    return 4;
  }
  if (cgpui::renderer_backend_target_name(cgpui::RendererBackendTarget::metal) !=
      std::string_view{"Metal"}) {
    return 5;
  }

  const std::string platform_target_text =
      read_source("include/cgpui/platform/target.hpp");
  if (platform_target_text.empty()) {
    return 6;
  }
  if (!contains(platform_target_text, "enum class DesktopPlatformTarget")) {
    return 7;
  }
  if (!contains(platform_target_text, "windows") ||
      !contains(platform_target_text, "linux_wayland") ||
      !contains(platform_target_text, "macos_cocoa")) {
    return 8;
  }

  const std::string platform_text =
      read_source("include/cgpui/platform/platform.hpp");
  if (platform_text.empty()) {
    return 20;
  }
  if (!contains(platform_text, "#include \"cgpui/platform/target.hpp\"")) {
    return 21;
  }

  const std::string native_surface_text =
      read_source("include/cgpui/platform/native_surface.hpp");
  if (native_surface_text.empty()) {
    return 9;
  }
  if (!contains(native_surface_text, "MetalSurfaceHandle") ||
      !contains(native_surface_text, "NativeSurfaceHandle")) {
    return 9;
  }

  const std::string renderer_text =
      read_source("include/cgpui/renderer/renderer.hpp");
  if (renderer_text.empty()) {
    return 10;
  }
  if (!contains(renderer_text,
                "#include \"cgpui/renderer/renderer_types.hpp\"")) {
    return 11;
  }

  const std::string renderer_types_text =
      read_source("include/cgpui/renderer/renderer_types.hpp");
  if (renderer_types_text.empty()) {
    return 22;
  }
  if (!contains(renderer_types_text, "enum class RendererBackendTarget")) {
    return 23;
  }
  if (!contains(renderer_types_text, "vulkan") ||
      !contains(renderer_types_text, "metal")) {
    return 12;
  }

  const std::string xmake_text = read_source("xmake.lua");
  if (xmake_text.empty()) {
    return 13;
  }
  const std::string macos_xmake_text =
      read_source("build/xmake/phase_h_macos.lua");
  if (!contains(xmake_text, "includes(\"build/xmake/phase_h_macos.lua\")") ||
      !contains(macos_xmake_text, "target(\"cgpui_platform_macos\")") ||
      !contains(macos_xmake_text, "src/platform/macos/*.mm") ||
      !contains(macos_xmake_text,
                "add_frameworks(\"AppKit\", \"QuartzCore\")")) {
    return 14;
  }
  const std::string metal_xmake_text =
      read_source("build/xmake/phase_h_metal.lua");
  if (!contains(xmake_text, "includes(\"build/xmake/phase_h_metal.lua\")") ||
      !contains(metal_xmake_text, "target(\"cgpui_renderer_metal\")") ||
      !contains(metal_xmake_text, "src/renderer/metal/*.mm") ||
      !contains(metal_xmake_text,
                "add_frameworks(\"Metal\", \"QuartzCore\")")) {
    return 15;
  }
  if (!contains(xmake_text, "target(\"desktop_target_readiness_test\")") ||
      !contains(xmake_text, "tests/architecture/desktop_target_readiness_test.cpp")) {
    return 16;
  }

  const std::string audit_text = read_source("docs/platform-mac-readiness.md");
  if (audit_text.empty()) {
    return 17;
  }
  if (!contains(audit_text, "Windows: Win32 + Vulkan") ||
      !contains(audit_text, "Linux: Wayland + Vulkan") ||
      !contains(audit_text, "macOS: Cocoa + Metal")) {
    return 18;
  }
  if (!contains(audit_text, "Not in scope") ||
      !contains(audit_text, "Step 88")) {
    return 19;
  }

  const std::string parity_text = read_source("docs/gpui-core-api-parity.md");
  if (parity_text.empty()) {
    return 30;
  }
  if (!contains(parity_text, "# GPUI-Core API Parity Audit") ||
      !contains(parity_text, "Windows/Linux Scope") ||
      !contains(parity_text, "Not Full Upstream GPUI Parity")) {
    return 31;
  }
  if (!contains(parity_text, "Implemented") ||
      !contains(parity_text, "Partial") ||
      !contains(parity_text, "Missing") ||
      !contains(parity_text, "Mac/Metal Deferred")) {
    return 32;
  }
  if (!contains(parity_text, "Context<T>") ||
      !contains(parity_text, "Entity<T>") ||
      !contains(parity_text, "WindowOptions") ||
      !contains(parity_text, "button") ||
      !contains(parity_text, "label") ||
      !contains(parity_text, "text_input") ||
      !contains(parity_text, "scrollable_list")) {
    return 33;
  }
  if (!contains(parity_text, "Vulkan") ||
      !contains(parity_text, "Wayland") ||
      !contains(parity_text, "Win32") ||
      !contains(parity_text, "Metal")) {
    return 34;
  }
  if (!contains(parity_text, "Next Milestone")) {
    return 35;
  }

  return 0;
}

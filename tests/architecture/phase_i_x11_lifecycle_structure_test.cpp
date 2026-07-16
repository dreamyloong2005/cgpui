#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (char value : text) lines += value == '\n';
  return lines + (!text.empty() && text.back() != '\n');
}
}  // namespace

int main() {
  constexpr const char* files[] = {
      "src/platform/linux/x11/x11_internal.hpp",
      "src/platform/linux/x11/x11_application_internal.hpp",
      "src/platform/linux/x11/x11_application.cpp",
      "src/platform/linux/x11/x11_application_windows.cpp",
      "src/platform/linux/x11/x11_event_loop.cpp",
      "src/platform/linux/x11/x11_window_internal.hpp",
      "src/platform/linux/x11/x11_window.cpp",
      "src/platform/linux/x11/x11_window_lifecycle.cpp",
      "src/platform/linux/x11/x11_window_events.cpp",
      "src/platform/linux/x11/x11_scale_internal.hpp",
      "src/platform/linux/x11/x11_scale.cpp",
  };
  for (const char* file : files) {
    const std::string source = read_source(file);
    if (source.empty() || line_count(source) > 220) return 1;
  }
  const std::string application = read_source(files[1]);
  const std::string event_loop = read_source(files[4]);
  const std::string window = read_source(files[5]);
  const std::string window_source = read_source(files[6]);
  if (!contains(application, "class X11Application final") ||
      !contains(application, "xcb_connection_t*") ||
      !contains(event_loop, "poll(") ||
      !contains(event_loop, "wakeup_requested()") ||
      !contains(window_source, "WindowWakeupRequested") ||
      !contains(window, "class X11Window final") ||
      !contains(window, "PlatformWindowCloseController")) {
    return 2;
  }
  const std::string vulkan =
      read_source("src/renderer/vulkan/vulkan_instance_surface.cpp");
  const std::string vulkan_internal =
      read_source("src/renderer/vulkan/vulkan_platform_internal.hpp");
  if (!contains(vulkan, "VK_KHR_XCB_SURFACE_EXTENSION_NAME") ||
      !contains(vulkan, "vkCreateXcbSurfaceKHR") ||
      !contains(vulkan_internal, "require_x11_surface")) {
    return 3;
  }
  const std::string module =
      read_source("build/xmake/phase_i_linux_backends.lua");
  const std::string targets =
      read_source("build/xmake/phase_i_structure_targets.lua");
  if (!contains(module, "target(\"x11_window_lifecycle_test\")") ||
      !contains(module, "target(\"x11_vulkan_surface_test\")") ||
      !contains(module, "target(\"x11_scale_test\")") ||
      !contains(targets,
                "target(\"phase_i_x11_lifecycle_structure_test\")")) {
    return 4;
  }
  return 0;
}

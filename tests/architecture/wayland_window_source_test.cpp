#include <fstream>
#include <iterator>
#include <algorithm>
#include <string>

namespace {

std::string read_wayland_source() {
  std::ifstream source("src/platform/linux/wayland_application.cpp");
  if (!source) {
    source.open("../../../../src/platform/linux/wayland_application.cpp");
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

std::string read_xmake_source() {
  std::ifstream source("xmake.lua");
  if (!source) {
    source.open("../../../../xmake.lua");
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

std::string linux_target_block(const std::string& xmake_text, const char* target_name) {
  const std::string target_marker =
      std::string("target(\"") + target_name + "\")";
  const auto target_position = xmake_text.find(target_marker);
  if (target_position == std::string::npos) {
    return {};
  }

  const auto next_target_position = xmake_text.find("\ntarget(\"", target_position + 1);
  const auto next_platform_position =
      xmake_text.find("\nif is_plat(\"macosx\")", target_position + 1);
  auto end_position = std::string::npos;
  if (next_target_position != std::string::npos &&
      next_platform_position != std::string::npos) {
    end_position = std::min(next_target_position, next_platform_position);
  } else if (next_target_position != std::string::npos) {
    end_position = next_target_position;
  } else {
    end_position = next_platform_position;
  }

  return xmake_text.substr(
      target_position,
      end_position == std::string::npos
          ? std::string::npos
          : end_position - target_position);
}

} // namespace

int main() {
  const std::string text = read_wayland_source();
  if (text.empty()) {
    return 2;
  }

  if (contains(text, "Wayland surface creation is not implemented yet")) {
    return 1;
  }

  if (!contains(text, "WaylandWindow") ||
      !contains(text, "xdg_wm_base") ||
      !contains(text, "xdg_surface") ||
      !contains(text, "xdg_toplevel")) {
    return 3;
  }

  if (!contains(text, "WaylandSurfaceHandle{.display = display_")) {
    return 4;
  }

  const std::string xmake_text = read_xmake_source();
  if (xmake_text.empty()) {
    return 5;
  }

  const std::string close_target_text =
      linux_target_block(xmake_text, "wayland_compositor_close_test");
  if (close_target_text.empty()) {
    return 6;
  }
  if (!contains(close_target_text, "tests/platform/wayland_compositor_close_test.cpp")) {
    return 7;
  }
  if (!contains(close_target_text, "wayland-server")) {
    return 8;
  }
  if (!contains(close_target_text, "add_tests(\"default\")")) {
    return 9;
  }
  if (!contains(close_target_text, "tests/platform/wayland_test_compositor.cpp")) {
    return 10;
  }

  const std::string resize_target_text =
      linux_target_block(xmake_text, "wayland_compositor_resize_test");
  if (resize_target_text.empty()) {
    return 11;
  }
  if (!contains(resize_target_text, "tests/platform/wayland_compositor_resize_test.cpp")) {
    return 12;
  }
  if (!contains(resize_target_text, "tests/platform/wayland_test_compositor.cpp")) {
    return 13;
  }
  if (!contains(resize_target_text, "wayland-server")) {
    return 14;
  }
  if (!contains(resize_target_text, "add_tests(\"default\")")) {
    return 15;
  }

  const std::string vulkan_target_text =
      linux_target_block(xmake_text, "wayland_vulkan_surface_test");
  if (vulkan_target_text.empty()) {
    return 16;
  }
  if (!contains(vulkan_target_text, "tests/renderer/wayland_vulkan_surface_test.cpp")) {
    return 17;
  }
  if (!contains(vulkan_target_text, "cgpui_platform_linux_wayland")) {
    return 18;
  }
  if (!contains(vulkan_target_text, "cgpui_renderer_vulkan")) {
    return 19;
  }
  if (!contains(vulkan_target_text, "\"vulkan\"")) {
    return 20;
  }
  if (!contains(vulkan_target_text, "add_tests(\"default\")")) {
    return 21;
  }

  return 0;
}

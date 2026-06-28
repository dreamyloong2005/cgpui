#include <fstream>
#include <iterator>
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

  const auto target_position =
      xmake_text.find("target(\"wayland_compositor_close_test\")");
  if (target_position == std::string::npos) {
    return 6;
  }

  const auto next_platform_position =
      xmake_text.find("if is_plat(\"macosx\")", target_position);
  const std::string target_text = xmake_text.substr(
      target_position,
      next_platform_position == std::string::npos
          ? std::string::npos
          : next_platform_position - target_position);
  if (!contains(target_text, "tests/platform/wayland_compositor_close_test.cpp")) {
    return 7;
  }
  if (!contains(target_text, "wayland-server")) {
    return 8;
  }
  if (!contains(target_text, "add_tests(\"default\")")) {
    return 9;
  }

  return 0;
}

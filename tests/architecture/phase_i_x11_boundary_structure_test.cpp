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

}  // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string backends =
      read_source("build/xmake/phase_i_linux_backends.lua");
  const std::string targets =
      read_source("build/xmake/phase_i_structure_targets.lua");
  if (!contains(xmake,
                "includes(\"build/xmake/phase_i_linux_backends.lua\")") ||
      !contains(xmake,
                "includes(\"build/xmake/phase_i_structure_targets.lua\")") ||
      backends.empty() || targets.empty()) {
    return 1;
  }

  constexpr const char* boundary_files[] = {
      "src/platform/linux/linux_backend_selection_internal.hpp",
      "src/platform/linux/linux_backend_selection.cpp",
      "src/platform/linux/linux_application_factory.cpp",
      "src/platform/linux/x11/x11_application_internal.hpp",
  };
  for (const char* file : boundary_files) {
    if (read_source(file).empty()) return 2;
  }

  const std::string selection = read_source(boundary_files[0]);
  if (!contains(selection, "enum class LinuxPlatformBackend") ||
      !contains(selection, "wayland") || !contains(selection, "x11") ||
      !contains(selection, "select_linux_platform_backend")) {
    return 3;
  }

  const std::string selection_source = read_source(boundary_files[1]);
  if (!contains(selection_source, "CGPUI_LINUX_BACKEND") ||
      !contains(selection_source, "WAYLAND_DISPLAY") ||
      !contains(selection_source, "DISPLAY")) {
    return 4;
  }

  const std::string factory = read_source(boundary_files[2]);
  const std::string old_factory =
      read_source("src/platform/linux/wayland_application_factory.cpp");
  if (!contains(factory, "select_linux_platform_backend") ||
      !contains(factory, "create_wayland_application") ||
      !contains(factory, "create_x11_application") ||
      contains(old_factory, "create_platform_application(")) {
    return 5;
  }

  if (!contains(backends, "target(\"cgpui_platform_linux_wayland\")") ||
      !contains(backends, "target(\"cgpui_platform_linux_x11\")") ||
      !contains(backends, "src/platform/linux/x11/*.cpp") ||
      !contains(backends, "add_syslinks(\"xcb\"") ||
      !contains(backends, "libxkbcommon")) {
    return 6;
  }

  if (!contains(targets,
                "target(\"phase_i_x11_boundary_structure_test\")") ||
      contains(xmake, "src/platform/linux/x11/*.cpp")) {
    return 7;
  }

  const std::string public_application =
      read_source("include/cgpui/platform/platform_application.hpp");
  const std::string public_window =
      read_source("include/cgpui/platform/platform_window.hpp");
  if (contains(public_application, "X11") || contains(public_window, "X11")) {
    return 8;
  }

  return 0;
}

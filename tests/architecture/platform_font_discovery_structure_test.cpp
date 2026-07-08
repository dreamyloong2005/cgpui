#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::filesystem::path executable_directory;

std::string read_file(const std::filesystem::path& path) {
  std::ifstream source(path);
  if (!source) {
    return {};
  }
  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

std::string read_source_from_project_root(
    std::filesystem::path directory,
    const std::filesystem::path& relative_path) {
  directory = std::filesystem::absolute(directory);
  for (; !directory.empty(); directory = directory.parent_path()) {
    if (std::filesystem::exists(directory / ".git")) {
      const std::string text = read_file(directory / relative_path);
      if (!text.empty()) {
        return text;
      }
    }
    if (directory == directory.root_path()) {
      break;
    }
  }
  return {};
}

std::string read_source(const char* path) {
  const std::filesystem::path relative_path(path);
  if (const char* source_root = std::getenv("CGPUI_SOURCE_ROOT");
      source_root != nullptr) {
    const std::string text =
        read_file(std::filesystem::path(source_root) / relative_path);
    if (!text.empty()) {
      return text;
    }
  }
  std::string text =
      read_source_from_project_root(executable_directory, relative_path);
  if (!text.empty()) {
    return text;
  }
  return read_source_from_project_root(
      std::filesystem::current_path(),
      relative_path);
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

} // namespace

int main(int argc, char** argv) {
  if (argc > 0 && argv[0] != nullptr) {
    executable_directory = std::filesystem::absolute(argv[0]).parent_path();
  }

  const std::string public_leaf =
      read_source("include/cgpui/platform/platform_font_discovery.hpp");
  const std::string platform_application =
      read_source("include/cgpui/platform/platform_application.hpp");
  const std::string platform_source =
      read_source("src/platform/platform_font_discovery.cpp");
  const std::string empty_source = read_source("src/platform/empty.cpp");
  const std::string win32_internal =
      read_source("src/platform/win32/win32_internal.hpp");
  const std::string win32_application =
      read_source("src/platform/win32/win32_application.cpp");
  const std::string win32_font =
      read_source("src/platform/win32/win32_font_discovery.cpp");
  const std::string wayland_application_core =
      read_source("src/platform/linux/wayland_application_core_internal.hpp");
  const std::string wayland_services =
      read_source("src/platform/linux/wayland_application_services.cpp");
  const std::string wayland_font =
      read_source("src/platform/linux/wayland_font_discovery.cpp");
  const std::string wayland_font_internal =
      read_source("src/platform/linux/wayland_font_discovery_internal.hpp");
  const std::string wayland_fontconfig =
      read_source("src/platform/linux/wayland_fontconfig_discovery.cpp");

  if (public_leaf.empty() || platform_application.empty() ||
      platform_source.empty() || empty_source.empty() ||
      win32_internal.empty() || win32_application.empty() ||
      win32_font.empty() || wayland_application_core.empty() ||
      wayland_services.empty() || wayland_font.empty() ||
      wayland_font_internal.empty() || wayland_fontconfig.empty()) {
    return 1;
  }

  if (!contains(public_leaf, "enum class PlatformFontDiscoveryBackend") ||
      !contains(public_leaf, "enum class PlatformFontDiscoveryStatus") ||
      !contains(public_leaf, "struct PlatformFontDiscoveryDiagnostics") ||
      !contains(public_leaf, "class PlatformFontDiscoveryResult") ||
      !contains(public_leaf, "make_platform_font_discovery_result(")) {
    return 2;
  }
  if (!contains(platform_application, "platform_font_discovery.hpp") ||
      !contains(platform_application, "discover_font_discovery()") ||
      contains(platform_application, "\"cgpui/ui/text.hpp\"")) {
    return 3;
  }
  if (!contains(platform_source, "make_platform_font_discovery_result(") ||
      !contains(platform_source, "PlatformApplication::discover_font_discovery()") ||
      !contains(platform_source, "PlatformApplication::discover_font_records()")) {
    return 4;
  }
  if (!contains(platform_source, "PlatformFontDiscoveryStatus::unsupported") ||
      contains(empty_source, "FontFaceDescriptor{")) {
    return 5;
  }
  if (!contains(win32_internal, "win32_discover_fonts()") ||
      !contains(win32_application, "discover_font_discovery()") ||
      !contains(win32_application, "win32_discover_fonts()") ||
      contains(win32_application, "FontFaceDescriptor{") ||
      contains(win32_application, "win32://Segoe UI")) {
    return 6;
  }
  if (!contains(win32_font, "win32_discover_fonts()") ||
      !contains(win32_font, "#include <dwrite.h>") ||
      !contains(win32_font, "DWriteCreateFactory") ||
      !contains(win32_font, "GetSystemFontCollection") ||
      !contains(win32_font, "GetFontFamilyCount") ||
      !contains(win32_font, "PlatformFontDiscoveryBackend::direct_write") ||
      !contains(win32_font, "PlatformFontDiscoveryStatus::native_available") ||
      !contains(win32_font, "PlatformFontDiscoveryStatus::deterministic_fallback") ||
      !contains(win32_font, "Segoe UI") ||
      !contains(win32_font, "win32://Segoe UI")) {
    return 7;
  }
  if (!contains(wayland_application_core, "discover_font_discovery()") ||
      contains(wayland_services, "discover_font_records(") ||
      contains(wayland_services, "FontFaceDescriptor{") ||
      contains(wayland_services, "fontconfig://sans-serif")) {
    return 8;
  }
  if (!contains(wayland_font, "WaylandApplication::discover_font_discovery(") ||
      !contains(wayland_font, "wayland_discover_fonts_with_fontconfig()") ||
      !contains(wayland_font, "wayland_deterministic_font_fallback()") ||
      !contains(wayland_font, "PlatformFontDiscoveryBackend::fontconfig") ||
      !contains(
          wayland_font,
          "PlatformFontDiscoveryStatus::deterministic_fallback") ||
      !contains(wayland_font, "fontconfig://sans-serif")) {
    return 9;
  }
  if (!contains(wayland_font_internal,
                "wayland_discover_fonts_with_fontconfig()") ||
      !contains(wayland_font_internal,
                "wayland_deterministic_font_fallback()")) {
    return 10;
  }
  if (!contains(wayland_fontconfig, "CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND") ||
      !contains(wayland_fontconfig, "#include <fontconfig/fontconfig.h>") ||
      !contains(wayland_fontconfig, "FcFontList") ||
      !contains(wayland_fontconfig, "FC_FAMILY") ||
      !contains(wayland_fontconfig, "FC_FILE") ||
      !contains(wayland_fontconfig,
                "PlatformFontDiscoveryStatus::native_available") ||
      !contains(wayland_fontconfig,
                "wayland_deterministic_font_fallback()")) {
    return 11;
  }

  return 0;
}

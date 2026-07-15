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

bool missing(const std::string& text, const char* value) {
  return !contains(text, value);
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char character : text) {
    lines += character == '\n';
  }
  return lines + (!text.empty() && text.back() != '\n');
}

}  // namespace

int main() {
  const std::string root = std::getenv("CGPUI_SOURCE_ROOT") == nullptr
      ? "."
      : std::getenv("CGPUI_SOURCE_ROOT");
  const std::string application = read_source("src/platform/macos/macos_application.mm");
  if (application.empty() || line_count(application) > 120 ||
      contains(application, "[window retain]") ||
      contains(application, "[window release]") ||
      contains(application, "[delegate release]") ||
      contains(application, "[[NSWindow alloc]") ||
      missing(application, "MacOSApplication")) {
    return 1;
  }

  constexpr const char* files[] = {
      "src/platform/macos/macos_application_internal.hpp",
      "src/platform/macos/macos_application_delegate.mm",
      "src/platform/macos/macos_event_loop.mm",
      "src/platform/macos/macos_window_internal.hpp",
      "src/platform/macos/macos_window.mm",
      "src/platform/macos/macos_window_factory.mm",
      "src/platform/macos/macos_window_delegate.mm",
      "src/platform/macos/macos_window_lifecycle.mm",
      "src/platform/macos/macos_window_cursor.mm",
      "src/platform/macos/macos_window_wakeup.mm",
  };
  for (const char* file : files) {
    const std::string contents = read_source(file);
    if (contents.empty() || contains(contents, "[window retain]") ||
        contains(contents, "[window release]") ||
        contains(contents, "[delegate release]") ||
        contains(contents, "autorelease")) {
      return 2;
    }
  }

  const std::string window = read_source("src/platform/macos/macos_window_internal.hpp");
  if (missing(window, "class MacOSWindow") ||
      missing(window, "PlatformWindowLifecycleState lifecycle_state() const override") ||
      missing(window, "PlatformWindowCloseState close_request_state() const override") ||
      missing(window, "std::function<void(MacOSWindow*)> unregister") ||
      missing(window, "__strong NSWindow* window_")) {
    return 3;
  }

  const std::string xmake = read_source("xmake.lua");
  const std::string module = read_source("build/xmake/phase_h_macos.lua");
  const std::string targets = read_source("build/xmake/phase_h_structure_targets.lua");
  if (missing(xmake, "includes(\"build/xmake/phase_h_macos.lua\")") ||
      missing(xmake, "includes(\"build/xmake/phase_h_structure_targets.lua\")") ||
      module.empty() ||
      missing(module, "target(\"macos_window_lifecycle_test\")") ||
      missing(module, "add_files(path.join(os.projectdir(), \"src/platform/macos/*.mm\"))") ||
      missing(module, "add_frameworks(\"AppKit\", \"QuartzCore\")") ||
      targets.empty() || missing(targets, "target(\"phase_h_cocoa_lifecycle_structure_test\")")) {
    return 6;
  }

  const std::string native = read_source("tests/platform/macos/macos_window_lifecycle_test.mm");
  if (native.empty() || missing(native, "resolve_close_request") ||
      missing(native, "request_wakeup_after") || missing(native, "cancel_wakeup_after")) {
    return 5;
  }
  (void)root;
  return 0;
}

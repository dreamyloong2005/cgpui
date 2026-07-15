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
  for (const char character : text) lines += character == '\n';
  return lines + (!text.empty() && text.back() != '\n');
}

}  // namespace

int main() {
  constexpr const char* files[] = {
      "src/platform/macos/macos_content_view.mm",
      "src/platform/macos/macos_input_internal.hpp",
      "src/platform/macos/macos_pointer.mm",
      "src/platform/macos/macos_scroll.mm",
      "src/platform/macos/macos_keyboard.mm",
  };
  for (const char* file : files) {
    const std::string contents = read_source(file);
    if (contents.empty() || contains(contents, "autorelease") ||
        contains(contents, " retain]") || contains(contents, " release]") ||
        line_count(contents) > 180) {
      return 1;
    }
  }

  const std::string content_view = read_source(files[0]);
  if (!contains(content_view, "mouseMoved:") ||
      !contains(content_view, "mouseDragged:") ||
      !contains(content_view, "scrollWheel:") ||
      !contains(content_view, "keyDown:") ||
      !contains(content_view, "keyUp:") ||
      !contains(content_view, "resetCursorRects")) {
    return 2;
  }
  constexpr const char* composition_files[] = {
      "src/platform/macos/macos_window.mm",
      "src/platform/macos/macos_window_factory.mm",
      "src/platform/macos/macos_window_cursor.mm",
  };
  for (const char* file : composition_files) {
    const std::string contents = read_source(file);
    if (contains(contents, "mouseMoved:") || contains(contents, "scrollWheel:") ||
        contains(contents, "keyDown:")) {
      return 7;
    }
  }

  const std::string input = read_source(files[1]);
  if (!contains(input, "macos_event_position_in_framebuffer") ||
      !contains(input, "macos_translate_key_event") ||
      !contains(input, "macos_translate_scroll_event")) {
    return 3;
  }

  const std::string pointer = read_source(files[2]);
  if (!contains(pointer, "addLocalMonitorForEventsMatchingMask") ||
      !contains(pointer, "PointerCaptureChanged")) {
    return 4;
  }

  const std::string window =
      read_source("src/platform/macos/macos_window_internal.hpp");
  if (!contains(window, "pointer_capture_state() const override") ||
      !contains(window, "set_pointer_capture(bool captured) override") ||
      !contains(window, "CGPUIMacOSContentView")) {
    return 5;
  }

  const std::string module = read_source("build/xmake/phase_h_macos.lua");
  const std::string targets =
      read_source("build/xmake/phase_h_structure_targets.lua");
  if (!contains(module, "target(\"macos_input_event_test\")") ||
      !contains(targets, "target(\"phase_h_macos_input_structure_test\")")) {
    return 6;
  }
  return 0;
}

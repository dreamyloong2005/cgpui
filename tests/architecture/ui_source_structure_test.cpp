#include <fstream>
#include <iterator>
#include <string>
#include <vector>

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

std::size_t line_count(const std::string& text) {
  std::size_t count = 0;
  for (const char value : text) {
    if (value == '\n') {
      count += 1;
    }
  }
  return count;
}

} // namespace

int main() {
  const std::vector<const char*> public_headers{
      "include/cgpui/ui/paint.hpp",
      "include/cgpui/ui/view.hpp",
      "include/cgpui/ui/runtime.hpp",
  };
  for (const char* header : public_headers) {
    if (read_source(header).empty()) {
      return 1;
    }
  }

  const std::string ui_header = read_source("include/cgpui/ui/ui.hpp");
  if (ui_header.empty()) {
    return 2;
  }
  if (!contains(ui_header, "#include \"cgpui/ui/paint.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/view.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/runtime.hpp\"")) {
    return 3;
  }
  if (contains(ui_header, "class PaintList") ||
      contains(ui_header, "class WindowRuntime") ||
      contains(ui_header, "struct WindowRuntimeContext")) {
    return 4;
  }

  const std::string paint_header = read_source("include/cgpui/ui/paint.hpp");
  if (!contains(paint_header, "class PaintList") ||
      !contains(paint_header, "struct PaintCommand") ||
      !contains(paint_header, "struct ImagePaint")) {
    return 5;
  }

  const std::string view_header = read_source("include/cgpui/ui/view.hpp");
  if (!contains(view_header, "class View") ||
      !contains(view_header, "using ViewContext = WindowRuntimeContext")) {
    return 6;
  }

  const std::string runtime_header =
      read_source("include/cgpui/ui/runtime.hpp");
  if (!contains(runtime_header, "class WindowRuntime") ||
      !contains(runtime_header, "struct WindowRuntimeContext") ||
      !contains(runtime_header, "struct AppContext")) {
    return 7;
  }

  const std::vector<const char*> source_files{
      "src/ui/paint.cpp",
      "src/ui/render_view.cpp",
      "src/ui/view.cpp",
      "src/ui/runtime_core.cpp",
      "src/ui/runtime_events.cpp",
      "src/ui/runtime_scheduling.cpp",
      "src/ui/runtime_diagnostics.cpp",
      "src/ui/runtime_context.cpp",
      "src/ui/app_context.cpp",
  };
  for (const char* source : source_files) {
    if (read_source(source).empty()) {
      return 8;
    }
  }

  const std::string ui_source = read_source("src/ui/ui.cpp");
  if (ui_source.empty()) {
    return 9;
  }
  if (line_count(ui_source) > 250) {
    return 10;
  }
  if (contains(ui_source, "PaintList::") ||
      contains(ui_source, "Result<RenderRecord> render_view") ||
      contains(ui_source, "WindowRuntime::run") ||
      contains(ui_source, "WindowRuntimeContext::") ||
      contains(ui_source, "AppContext::")) {
    return 11;
  }

  return 0;
}

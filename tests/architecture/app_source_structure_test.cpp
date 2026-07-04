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
  const std::string cgpui_header = read_source("include/cgpui/cgpui.hpp");
  const std::string prelude_header = read_source("include/cgpui/prelude.hpp");
  const std::string app_header = read_source("include/cgpui/app/app.hpp");
  const std::string application_header =
      read_source("include/cgpui/app/application.hpp");
  const std::string app_facade_header =
      read_source("include/cgpui/app/app_facade.hpp");
  const std::string window_header = read_source("include/cgpui/app/window.hpp");
  const std::string application_source = read_source("src/app/application.cpp");
  const std::string app_facade_source =
      read_source("src/app/app_facade.cpp");
  const std::string window_source = read_source("src/app/window.cpp");
  const std::string app_context_source =
      read_source("src/app/app_context_facade.cpp");

  if (cgpui_header.empty() || prelude_header.empty() || app_header.empty() ||
      application_header.empty() || app_facade_header.empty() ||
      window_header.empty() || application_source.empty() ||
      app_facade_source.empty() || window_source.empty() ||
      app_context_source.empty()) {
    return 1;
  }
  if (!contains(cgpui_header, "#include \"cgpui/prelude.hpp\"") ||
      contains(cgpui_header, "#include \"cgpui/app/app.hpp\"") ||
      line_count(cgpui_header) > 8) {
    return 2;
  }
  if (!contains(prelude_header, "#include \"cgpui/app/app.hpp\"") ||
      !contains(prelude_header, "#include \"cgpui/ui/ui.hpp\"") ||
      !contains(prelude_header, "#include \"cgpui/renderer/renderer.hpp\"") ||
      !contains(prelude_header, "#include \"cgpui/platform/platform.hpp\"") ||
      line_count(prelude_header) > 40) {
    return 13;
  }
  if (!contains(app_header, "#include \"cgpui/app/application.hpp\"")) {
    return 3;
  }
  if (line_count(app_header) > 20 ||
      contains(app_header, "class Application")) {
    return 4;
  }
  if (!contains(app_header, "#include \"cgpui/app/app_facade.hpp\"") ||
      !contains(app_header, "#include \"cgpui/app/window.hpp\"") ||
      contains(app_header, "class App") ||
      contains(app_header, "class Window")) {
    return 9;
  }
  if (!contains(application_header, "class Application") ||
      !contains(application_header, "static Result<Application> create()") ||
      !contains(application_header, "int run(") ||
      !contains(application_header, "PlatformApplication& platform_application()")) {
    return 5;
  }
  if (line_count(application_header) > 120) {
    return 6;
  }
  if (!contains(application_source, "Application::create()") ||
      !contains(application_source, "create_platform_application()") ||
      !contains(application_source, "Application::run(") ||
      !contains(application_source, "run_app(")) {
    return 7;
  }
  if (line_count(application_source) > 120) {
    return 8;
  }
  if (!contains(app_facade_header, "class App") ||
      !contains(app_facade_header, "open_window(") ||
      !contains(app_facade_header, "root_window()") ||
      !contains(app_facade_header, "std::optional<Window> window(") ||
      line_count(app_facade_header) > 120) {
    return 10;
  }
  if (!contains(window_header, "class Window") ||
      !contains(window_header, "runtime_id()") ||
      !contains(window_header, "descriptor()") ||
      !contains(window_header, "viewport_size()") ||
      !contains(window_header, "scale()") ||
      line_count(window_header) > 120) {
    return 11;
  }
  if (!contains(app_facade_source, "App::open_window(") ||
      !contains(app_facade_source, "runtime_->open_window(") ||
      !contains(window_source, "Window::descriptor()") ||
      !contains(window_source, "Window::viewport_size()") ||
      !contains(app_context_source, "AppContext::app()") ||
      !contains(app_context_source, "WindowRuntimeContext::app_context()") ||
      !contains(app_context_source, "WindowRuntimeContext::window()") ||
      !contains(app_context_source, "WindowRuntimeContext::current_window()")) {
    return 12;
  }
  return 0;
}

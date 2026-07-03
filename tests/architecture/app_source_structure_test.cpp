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
  const std::string app_header = read_source("include/cgpui/app/app.hpp");
  const std::string application_header =
      read_source("include/cgpui/app/application.hpp");
  const std::string application_source = read_source("src/app/application.cpp");

  if (cgpui_header.empty() || app_header.empty() ||
      application_header.empty() || application_source.empty()) {
    return 1;
  }
  if (!contains(cgpui_header, "#include \"cgpui/app/app.hpp\"")) {
    return 2;
  }
  if (!contains(app_header, "#include \"cgpui/app/application.hpp\"")) {
    return 3;
  }
  if (line_count(app_header) > 20 ||
      contains(app_header, "class Application")) {
    return 4;
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
  return 0;
}

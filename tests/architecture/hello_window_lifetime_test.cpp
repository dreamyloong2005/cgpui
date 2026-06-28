#include <fstream>
#include <iterator>
#include <string>

int main() {
  std::ifstream source("examples/hello_window/main.cpp");
  if (!source) {
    source.open("../../../../examples/hello_window/main.cpp");
  }
  if (!source) {
    return 2;
  }

  const std::string text{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};

  const auto window_position =
      text.find("std::unique_ptr<cgpui::PlatformWindow> window;");
  const auto renderer_position =
      text.find("std::unique_ptr<cgpui::Renderer> renderer;");
  if (window_position == std::string::npos ||
      renderer_position == std::string::npos) {
    return 3;
  }

  return window_position < renderer_position ? 0 : 1;
}

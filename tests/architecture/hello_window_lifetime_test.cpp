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

  if (window_position > renderer_position) {
    return 1;
  }

  if (text.find("CGPUI_EXIT_AFTER_FIRST_FRAME") == std::string::npos) {
    return 4;
  }
  if (text.find("exit_after_first_frame") == std::string::npos) {
    return 5;
  }
  if (text.find("first_frame_presented") == std::string::npos) {
    return 6;
  }
  if (text.find("(*app)->quit();") == std::string::npos) {
    return 7;
  }
  if (text.find("CGPUI_RESIZE_AFTER_FIRST_FRAME") == std::string::npos) {
    return 8;
  }
  if (text.find("resize_after_first_frame") == std::string::npos) {
    return 9;
  }
  if (text.find("second_frame_presented") == std::string::npos) {
    return 10;
  }
  if (text.find("window->request_redraw();") == std::string::npos) {
    return 11;
  }

  return 0;
}

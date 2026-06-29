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

  if (text.find("cgpui::WindowRuntime runtime(") == std::string::npos) {
    return 3;
  }
  if (text.find("set_after_frame_callback") == std::string::npos) {
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
  if (text.find("context.application.quit();") == std::string::npos) {
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
  if (text.find("context.window.request_redraw();") == std::string::npos) {
    return 11;
  }
  if (text.find("CGPUI_CLOSE_AFTER_FIRST_FRAME") == std::string::npos) {
    return 12;
  }
  if (text.find("close_after_first_frame") == std::string::npos) {
    return 13;
  }
  if (text.find("close_requested_after_first_frame") == std::string::npos) {
    return 14;
  }
  if (text.find("context.window.request_close();") == std::string::npos) {
    return 15;
  }
  if (text.find("cgpui::ElementTree") == std::string::npos) {
    return 18;
  }
  if (text.find("cgpui::ElementBuilder::box()") == std::string::npos) {
    return 19;
  }
  if (text.find("cgpui::TextModel") == std::string::npos) {
    return 20;
  }
  if (text.find("runtime.set_element_root(") == std::string::npos) {
    return 21;
  }
  if (text.find("runtime.bind_text_model(") == std::string::npos) {
    return 22;
  }
  if (text.find("runtime.set_element_cursor(") == std::string::npos) {
    return 23;
  }
  if (text.find("runtime.register_action(") == std::string::npos) {
    return 24;
  }
  if (text.find("runtime.bind_key(") == std::string::npos) {
    return 25;
  }
  if (text.find("context.runtime.request_keyboard_focus(") ==
      std::string::npos) {
    return 26;
  }
  if (text.find("context.runtime.subscribe_view_to_entity(") ==
      std::string::npos) {
    return 27;
  }
  if (text.find("context.runtime.notify_entity_changed(") ==
      std::string::npos) {
    return 28;
  }
  if (text.find("context.runtime.request_layout();") == std::string::npos) {
    return 29;
  }
  if (text.find("CGPUI_DEMO_INJECT_TEXT") == std::string::npos) {
    return 30;
  }

  std::ifstream platform_source("include/cgpui/platform/platform.hpp");
  if (!platform_source) {
    platform_source.open("../../../../include/cgpui/platform/platform.hpp");
  }
  if (!platform_source) {
    return 16;
  }

  const std::string platform_text{
      std::istreambuf_iterator<char>(platform_source),
      std::istreambuf_iterator<char>()};
  if (platform_text.find("virtual void request_close() = 0;") ==
      std::string::npos) {
    return 17;
  }

  return 0;
}

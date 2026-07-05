#include <array>
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

} // namespace

int main() {
  const std::string example =
      read_source("examples/api_parity/public_api_compatibility/main.cpp");
  const std::string xmake = read_source("xmake.lua");

  if (example.empty() || xmake.empty()) {
    return 1;
  }
  if (!contains(xmake, "target(\"api_parity_public_api_compatibility\")") ||
      !contains(
          xmake,
          "examples/api_parity/public_api_compatibility/main.cpp")) {
    return 2;
  }

  constexpr std::array required_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicApiCompatibilityView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicApiCompatibilityView>&",
      "static_assert(cgpui::Render<PublicApiCompatibilityView>)",
      "cgpui::Application::create()",
      "cgpui::AppRunnerOptions",
      "cgpui::WindowOptions",
      "cgpui::App",
      "cgpui::Window",
      "cgpui::ViewHandle<",
      "cgpui::WeakViewHandle<",
      "cgpui::EntityHandle<",
      "cgpui::WeakEntity<",
      "try_open_window(",
      "try_install_native_menu(",
      "try_show_native_file_dialog(",
      "try_spawn_task(",
      "try_draw_frame(",
      "register_action(",
      "bind_key(",
      "capture_pointer(",
      "release_pointer(",
  };
  for (std::size_t index = 0; index < required_fragments.size(); ++index) {
    if (!contains(example, required_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array forbidden_fragments{
      "#include \"cgpui/ui/",
      "#include \"cgpui/platform/",
      "#include \"cgpui/renderer/",
      "#include \"src/",
      "#include \"../",
      "WindowRuntime",
      ".runtime",
      "runtime.",
      "window_runtime",
      "window_runtime_records",
      "app_opened_windows",
  };
  for (std::size_t index = 0; index < forbidden_fragments.size(); ++index) {
    if (contains(example, forbidden_fragments[index])) {
      return 50 + static_cast<int>(index);
    }
  }

  return 0;
}

#include "cgpui/cgpui.hpp"

#include <cstdlib>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>

class HelloWorldView final : public cgpui::View {
 public:
  explicit HelloWorldView(std::string text) : text_(std::move(text)) {}

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(cgpui::Context<HelloWorldView>& context) override {
    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .background(cgpui::rgb(80, 80, 80))
            .padding(cgpui::edges(24.0F))
            .gap(cgpui::px(12.0F))
            .child(cgpui::label("Hello, " + text_ + "!")
                       .font_size(24.0F)
                       .foreground(cgpui::rgb(255, 255, 255))
                       .build())
            .child(cgpui::div()
                       .size(96.0F, 32.0F)
                       .background(cgpui::rgb(0, 0, 255))
                       .border_width(cgpui::edges(1.0F))
                       .border_color(cgpui::rgb(255, 255, 255))
                       .border_radius(cgpui::BorderRadii::all(6.0F))));
  }

 private:
  std::string text_;
};

static_assert(cgpui::Render<HelloWorldView>);

int main() {
  // GPUI upstream hello_world.rs parity:
  // application().run -> Application::create + Application::run.
  // App::open_window -> AppRunnerOptions::window.
  // Render for HelloWorld -> Render<T> + Context<T> + IntoElement.
  // div()/px()/rgb()/child() keep their C++ public-prelude spelling.
  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  HelloWorldView view("World");
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI API Parity Hello World")
                       .size(cgpui::Size{500.0F, 500.0F})
                       .to_descriptor();
  options.setup_context = [](cgpui::AppContext& app_context) {
    app_context.runtime.set_after_frame_callback(
        [](const cgpui::ViewContext& context) {
          if (std::getenv("CGPUI_EXIT_AFTER_FIRST_FRAME") != nullptr) {
            context.application.quit();
          }
        });
  };

  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      options);
}

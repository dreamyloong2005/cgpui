#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <expected>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

struct RefreshAction {
  static constexpr std::string_view name = "public.refresh";
};

struct PublicApiCompatibilityModel {
  explicit PublicApiCompatibilityModel(int initial_revision)
      : revision(initial_revision) {}

  int revision = 0;
};

class PublicApiCompatibilityView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(cgpui::Context<PublicApiCompatibilityView>& context) override {
    const cgpui::App app = context.app();
    const cgpui::Window window = context.window();
    const cgpui::ViewHandle<PublicApiCompatibilityView> view =
        context.view<PublicApiCompatibilityView>();
    const cgpui::WeakViewHandle<PublicApiCompatibilityView> weak_view =
        view.downgrade();
    const cgpui::Model<PublicApiCompatibilityModel> model =
        context.new_model<PublicApiCompatibilityModel>(0);
    const cgpui::EntityHandle<PublicApiCompatibilityModel> entity =
        context.entity(model);
    const cgpui::WeakEntity<PublicApiCompatibilityModel> weak_entity =
        context.weak_entity(model);
    const cgpui::ElementId root_element{1};

    context.register_action(
        std::string(RefreshAction::name),
        [](const cgpui::ViewContext& action_context) {
          action_context.request_render();
          return cgpui::EventResult::consumed_event();
        });
    context.register_command_palette_entry<RefreshAction>(
        cgpui::CommandPaletteEntry{
            .title = "Refresh public API example",
            .group = "Examples",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-r",
        });
    (void)context.bind_key(
        "ctrl-r",
        std::string(RefreshAction::name),
        cgpui::KeyBindingContext::focused_element(root_element));
    context.capture_pointer(root_element);
    context.release_pointer(root_element);
    (void)context.try_spawn_task([](const cgpui::ViewContext&) {});

    (void)app;
    (void)window;
    (void)weak_view;
    (void)entity;
    (void)weak_entity;

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .padding(cgpui::edges(16.0F))
            .background(cgpui::rgb(28, 34, 42))
            .child(cgpui::label("Public API compatibility")
                       .font_size(18.0F)
                       .foreground(cgpui::rgb(236, 242, 248))
                       .build()));
  }
};

using AppRef = cgpui::App&;
using AppContextRef = cgpui::AppContext&;
using AsyncRef = cgpui::AsyncContextCapability&;
using TestRef = cgpui::TestContextCapability&;

static_assert(cgpui::Action<RefreshAction>);
static_assert(cgpui::action_name<RefreshAction>() == RefreshAction::name);
static_assert(cgpui::Render<PublicApiCompatibilityView>);
static_assert(std::is_same_v<
              decltype(std::declval<AppRef>().try_open_window(
                  std::declval<cgpui::WindowOptions>())),
              cgpui::Result<cgpui::AppOpenedWindow>>);
static_assert(std::is_same_v<
              decltype(std::declval<AppContextRef>().try_open_window(
                  std::declval<cgpui::WindowOptions>())),
              cgpui::Result<cgpui::AppOpenedWindow>>);
static_assert(std::is_same_v<
              decltype(std::declval<AppContextRef>().try_install_native_menu(
                  std::declval<cgpui::NativeMenuModel>())),
              cgpui::Result<cgpui::NativeMenuInstallation>>);
static_assert(std::is_same_v<
              decltype(std::declval<AppContextRef>().try_show_native_file_dialog(
                  std::declval<cgpui::NativeFileDialogOptions>())),
              cgpui::Result<cgpui::NativeFileDialogResult>>);
static_assert(std::is_same_v<
              decltype(std::declval<AsyncRef>().try_spawn_task(
                  std::declval<cgpui::TaskCompletionCallback>())),
              cgpui::Result<cgpui::TaskHandle>>);
static_assert(std::is_same_v<
              decltype(std::declval<TestRef>().try_draw_frame()),
              cgpui::Result<void>>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Public API Compatibility")
                       .size(420.0F, 240.0F)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_API_COMPATIBILITY") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicApiCompatibilityView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

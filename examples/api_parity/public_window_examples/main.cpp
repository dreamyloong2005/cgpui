#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

struct InstallMenusAction {
  static constexpr std::string_view name = "set_menus.install";
};

struct FocusInputAction {
  static constexpr std::string_view name = "input.focus";
};

struct PreviewWindowPositioningAction {
  static constexpr std::string_view name = "window_positioning.preview";
};

struct ToggleWindowShadowAction {
  static constexpr std::string_view name = "window_shadow.toggle";
};

class PublicWindowExamplesView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicWindowExamplesView>& context) override {
    install_actions(context);
    install_menu_demo(context);

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .bg(cgpui::rgb(22, 27, 33))
            .p(24.0F)
            .child(cgpui::v_flex()
                       .size_pct(100.0F, 100.0F)
                       .gap(12.0F)
                       .p(18.0F)
                       .rounded(8.0F)
                       .bg(cgpui::rgb(37, 45, 54))
                       .shadow(window_shadow())
                       .child(cgpui::label("Window examples")
                                  .font_size(20.0F)
                                  .foreground(cgpui::rgb(239, 243, 248))
                                  .build())
                       .child(cgpui::label("set_menus, shadow, window_positioning, window_shadow, input")
                                  .font_size(12.0F)
                                  .foreground(cgpui::rgb(184, 196, 209))
                                  .build())
                       .child(cgpui::h_flex()
                                  .gap(10.0F)
                                  .child(cgpui::button(
                                             InstallMenusAction::name)
                                             .label("Install menus")
                                             .build())
                                  .child(cgpui::button(
                                             ToggleWindowShadowAction::name)
                                             .label("Toggle shadow")
                                             .build()))
                       .child(cgpui::h_flex()
                                  .gap(10.0F)
                                  .child(cgpui::menu_item(
                                             PreviewWindowPositioningAction::name)
                                             .label("Preview window positioning")
                                             .selected(true)
                                             .build())
                                  .child(cgpui::menu_item(
                                             FocusInputAction::name)
                                             .label("Focus input example")
                                             .build()))
                       .child(cgpui::text_input(input_model_)
                                  .key("window-examples-input")
                                  .style(input_style())
                                  .foreground(cgpui::rgb(244, 247, 250))
                                  .font_size(14.0F)
                                  .build())
                       .child(cgpui::div()
                                  .fixed()
                                  .top(22.0F)
                                  .left(340.0F)
                                  .w(230.0F)
                                  .h(86.0F)
                                  .rounded(8.0F)
                                  .bg(cgpui::rgb(244, 248, 252))
                                  .shadow_sm()
                                  .p(12.0F)
                                  .child(cgpui::label("Window shadow preview")
                                             .font_size(13.0F)
                                             .foreground(cgpui::rgb(34, 43, 54))
                                             .build()))));
  }

 private:
  static cgpui::BoxShadow window_shadow() {
    return cgpui::BoxShadow{
        .color = cgpui::rgba(7, 12, 18, 0.34F),
        .offset = cgpui::Point{0.0F, 10.0F},
        .blur_radius = 28.0F,
        .spread_radius = 2.0F,
    };
  }

  static cgpui::Style input_style() {
    return cgpui::Style{}
        .with_preferred_size(cgpui::Size{420.0F, 42.0F})
        .with_background_color(cgpui::rgb(49, 59, 70))
        .with_border_width(cgpui::edges(1.0F))
        .with_border_color(cgpui::rgb(92, 111, 129))
        .with_border_radius(cgpui::BorderRadii::all(6.0F))
        .with_padding(cgpui::edges(12.0F, 8.0F));
  }

  void install_actions(
      cgpui::Context<PublicWindowExamplesView>& context) const {
    context.register_command_palette_entry<InstallMenusAction>(
        cgpui::CommandPaletteEntry{
            .title = "Install menu demo",
            .group = "Window Examples",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-m",
        });
    context.register_command_palette_entry<FocusInputAction>(
        cgpui::CommandPaletteEntry{
            .title = "Focus input example",
            .group = "Window Examples",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-i",
        });
    context.register_command_palette_entry<PreviewWindowPositioningAction>(
        cgpui::CommandPaletteEntry{
            .title = "Preview window positioning",
            .group = "Window Examples",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-p",
        });
    context.register_command_palette_entry<ToggleWindowShadowAction>(
        cgpui::CommandPaletteEntry{
            .title = "Toggle window shadow",
            .group = "Window Examples",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-s",
        });

    (void)context.bind_key(
        "ctrl-m",
        std::string(InstallMenusAction::name),
        cgpui::KeyBindingContext::window());
    (void)context.bind_key(
        "ctrl-i",
        std::string(FocusInputAction::name),
        cgpui::KeyBindingContext::window());
  }

  void install_menu_demo(
      cgpui::Context<PublicWindowExamplesView>& context) const {
    cgpui::NativeMenuModel menu{
        .items =
            {
                cgpui::NativeMenuItem{
                    .kind = cgpui::NativeMenuItemKind::submenu,
                    .title = "Examples",
                    .children =
                        {
                            cgpui::NativeMenuItem{
                                .title = "Install Menus",
                                .action_name =
                                    std::string(InstallMenusAction::name),
                                .accelerator =
                                    cgpui::NativeMenuAccelerator{
                                        .key_code = 'M',
                                        .action = cgpui::KeyAction::pressed,
                                        .modifiers = {.control = true},
                                    },
                            },
                            cgpui::NativeMenuItem{
                                .kind = cgpui::NativeMenuItemKind::separator,
                            },
                            cgpui::NativeMenuItem{
                                .title = "Focus Input",
                                .action_name =
                                    std::string(FocusInputAction::name),
                                .accelerator =
                                    cgpui::NativeMenuAccelerator{
                                        .key_code = 'I',
                                        .action = cgpui::KeyAction::pressed,
                                        .modifiers = {.control = true},
                                    },
                            },
                        },
                },
            },
    };
    (void)context.try_install_native_menu(std::move(menu));
  }

  cgpui::TextModel input_model_{"Type into the public input example"};
};

using WindowOptionsRef = cgpui::WindowOptions&;
using WindowExampleContextRef = cgpui::Context<PublicWindowExamplesView>&;

static_assert(cgpui::Action<InstallMenusAction>);
static_assert(cgpui::Action<FocusInputAction>);
static_assert(cgpui::Action<PreviewWindowPositioningAction>);
static_assert(cgpui::Action<ToggleWindowShadowAction>);
static_assert(cgpui::Render<PublicWindowExamplesView>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowOptionsRef>().decorations(true)),
                   cgpui::WindowOptions&>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowOptionsRef>().resizable(true)),
                   cgpui::WindowOptions&>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowOptionsRef>().transparent(false)),
                   cgpui::WindowOptions&>);
static_assert(std::is_same_v<decltype(std::declval<WindowExampleContextRef>()
                                          .try_install_native_menu(
                                              std::declval<cgpui::NativeMenuModel>())),
                             cgpui::Result<cgpui::NativeMenuInstallation>>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Window Examples")
                       .size(680.0F, 460.0F)
                       .decorations(true)
                       .resizable(true)
                       .transparent(false)
                       .titlebar_visible(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_WINDOW_EXAMPLES") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicWindowExamplesView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

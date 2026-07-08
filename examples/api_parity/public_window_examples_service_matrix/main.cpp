#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

struct InstallServiceMenuAction {
  static constexpr std::string_view name =
      "window_examples.service_matrix.menu";
};

struct FocusServiceInputAction {
  static constexpr std::string_view name =
      "window_examples.service_matrix.input";
};

struct PreviewServicePositionAction {
  static constexpr std::string_view name =
      "window_examples.service_matrix.position";
};

struct ToggleServiceShadowAction {
  static constexpr std::string_view name =
      "window_examples.service_matrix.shadow";
};

class PublicWindowExamplesServiceMatrixView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicWindowExamplesServiceMatrixView>& context)
      override {
    const cgpui::TestContextCapability test = context.test_context();
    install_actions(context);
    install_service_menu(context);
    drive_service_matrix(test);

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .bg(cgpui::rgb(17, 24, 31))
            .p(22.0F)
            .child(cgpui::v_stack()
                       .size_pct(100.0F, 100.0F)
                       .gap(12.0F)
                       .p(18.0F)
                       .rounded(8.0F)
                       .bg(cgpui::rgb(38, 48, 58))
                       .shadow(service_shadow())
                       .child(cgpui::label("window/examples service matrix")
                                  .font_size(19.0F)
                                  .foreground(cgpui::rgb(242, 246, 250))
                                  .build())
                       .child(cgpui::label(
                                  "menu accelerators and command palette service matrix")
                                  .font_size(12.0F)
                                  .foreground(cgpui::rgb(181, 194, 207))
                                  .build())
                       .child(cgpui::h_stack()
                                  .gap(10.0F)
                                  .child(cgpui::button(
                                             InstallServiceMenuAction::name)
                                             .label("Install service menu")
                                             .key("service-menu-button")
                                             .build())
                                  .child(cgpui::button(
                                             ToggleServiceShadowAction::name)
                                             .label("Toggle service shadow")
                                             .key("service-shadow-button")
                                             .build()))
                       .child(cgpui::h_stack()
                                  .gap(10.0F)
                                  .child(cgpui::menu_item(
                                             PreviewServicePositionAction::name)
                                             .label("Preview service position")
                                             .selected(true)
                                             .key("service-position-item")
                                             .build())
                                  .child(cgpui::menu_item(
                                             FocusServiceInputAction::name)
                                             .label("Focus service input")
                                             .key("service-input-item")
                                             .build()))
                       .child(cgpui::text_input(input_model_)
                                  .key("service-matrix-input")
                                  .style(input_style())
                                  .foreground(cgpui::rgb(245, 248, 251))
                                  .font_size(14.0F)
                                  .build())
                       .child(cgpui::div()
                                  .fixed()
                                  .top(24.0F)
                                  .left(392.0F)
                                  .w(250.0F)
                                  .h(92.0F)
                                  .rounded(8.0F)
                                  .bg(cgpui::rgb(247, 250, 253))
                                  .shadow_sm()
                                  .p(12.0F)
                                  .child(cgpui::label(
                                             "Window options, shadow, fixed positioning, and text input service examples")
                                             .font_size(12.0F)
                                             .foreground(cgpui::rgb(31, 40, 50))
                                             .build()))));
  }

 private:
  void install_actions(
      cgpui::Context<PublicWindowExamplesServiceMatrixView>& context) const {
    context.register_command_palette_entry<InstallServiceMenuAction>(
        cgpui::CommandPaletteEntry{
            .title = "Install service menu",
            .group = "Window Examples Service Matrix",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-alt-m",
        });
    context.register_command_palette_entry<FocusServiceInputAction>(
        cgpui::CommandPaletteEntry{
            .title = "Focus service input",
            .group = "Window Examples Service Matrix",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-alt-i",
        });
    context.register_command_palette_entry<PreviewServicePositionAction>(
        cgpui::CommandPaletteEntry{
            .title = "Preview service positioning",
            .group = "Window Examples Service Matrix",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-alt-p",
        });
    context.register_command_palette_entry<ToggleServiceShadowAction>(
        cgpui::CommandPaletteEntry{
            .title = "Toggle service shadow",
            .group = "Window Examples Service Matrix",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-alt-s",
        });

    (void)context.bind_key(
        "ctrl-alt-m",
        std::string(InstallServiceMenuAction::name),
        cgpui::KeyBindingContext::window());
    (void)context.bind_key(
        "ctrl-alt-i",
        std::string(FocusServiceInputAction::name),
        cgpui::KeyBindingContext::window());
    (void)context.bind_key(
        "ctrl-alt-p",
        std::string(PreviewServicePositionAction::name),
        cgpui::KeyBindingContext::window());
  }

  void install_service_menu(
      cgpui::Context<PublicWindowExamplesServiceMatrixView>& context) const {
    cgpui::KeyboardModifiers menu_modifiers{};
    menu_modifiers.control = true;
    menu_modifiers.alt = true;

    cgpui::NativeMenuItem install_item{};
    install_item.title = "Install Service Menu";
    install_item.action_name = std::string(InstallServiceMenuAction::name);
    install_item.accelerator = cgpui::NativeMenuAccelerator{
        .key_code = 'M',
        .action = cgpui::KeyAction::pressed,
        .modifiers = menu_modifiers,
    };

    cgpui::NativeMenuItem separator{};
    separator.kind = cgpui::NativeMenuItemKind::separator;

    cgpui::NativeMenuItem focus_item{};
    focus_item.title = "Focus Service Input";
    focus_item.action_name = std::string(FocusServiceInputAction::name);
    focus_item.accelerator = cgpui::NativeMenuAccelerator{
        .key_code = 'I',
        .action = cgpui::KeyAction::pressed,
        .modifiers = menu_modifiers,
    };

    cgpui::NativeMenuItem submenu{};
    submenu.kind = cgpui::NativeMenuItemKind::submenu;
    submenu.title = "Service Matrix";
    submenu.children = {std::move(install_item), std::move(separator),
                        std::move(focus_item)};

    cgpui::NativeMenuModel menu{
        .items = {std::move(submenu)},
    };
    (void)context.try_install_native_menu(std::move(menu));
  }

  static void drive_service_matrix(const cgpui::TestContextCapability& test) {
    test.dispatch_window_activation(true);
    test.dispatch_window_focus(true);
    test.request_redraw();
    (void)test.try_draw_frame();
    (void)test.simulate_keystrokes("ctrl-alt-m");
    test.dispatch_pointer_move(cgpui::Point{40.0F, 142.0F});
  }

  static cgpui::BoxShadow service_shadow() {
    return cgpui::BoxShadow{
        .color = cgpui::rgba(5, 10, 15, 0.30F),
        .offset = cgpui::Point{0.0F, 8.0F},
        .blur_radius = 22.0F,
        .spread_radius = 1.0F,
    };
  }

  static cgpui::Style input_style() {
    return cgpui::Style{}
        .with_preferred_size(cgpui::Size{360.0F, 40.0F})
        .with_background_color(cgpui::rgb(49, 60, 70))
        .with_border_width(cgpui::edges(1.0F))
        .with_border_color(cgpui::rgb(95, 113, 132))
        .with_border_radius(cgpui::BorderRadii::all(6.0F))
        .with_padding(cgpui::edges(12.0F, 8.0F));
  }

  cgpui::TextModel input_model_{"Service matrix input example"};
};

using ServiceMatrixContextRef =
    cgpui::Context<PublicWindowExamplesServiceMatrixView>&;
using ServiceMatrixTestRef = cgpui::TestContextCapability&;
using WindowOptionsRef = cgpui::WindowOptions&;

static_assert(cgpui::Action<InstallServiceMenuAction>);
static_assert(cgpui::Action<FocusServiceInputAction>);
static_assert(cgpui::Action<PreviewServicePositionAction>);
static_assert(cgpui::Action<ToggleServiceShadowAction>);
static_assert(cgpui::Render<PublicWindowExamplesServiceMatrixView>);
static_assert(
    std::is_same_v<decltype(std::declval<ServiceMatrixContextRef>()
                                .try_install_native_menu(
                                    std::declval<cgpui::NativeMenuModel>())),
                   cgpui::Result<cgpui::NativeMenuInstallation>>);
static_assert(
    std::is_same_v<decltype(std::declval<ServiceMatrixContextRef>()
                                .bind_key(
                                    std::declval<std::string_view>(),
                                    std::declval<std::string>(),
                                    cgpui::KeyBindingContext::window())),
                   bool>);
static_assert(
    std::is_same_v<decltype(std::declval<ServiceMatrixTestRef>()
                                .simulate_keystrokes(
                                    std::declval<std::string_view>())),
                   bool>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowOptionsRef>()
                                .titlebar_visible(true)),
                   cgpui::WindowOptions&>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Window Examples Service Matrix")
                       .size(760.0F, 500.0F)
                       .decorations(true)
                       .resizable(true)
                       .transparent(false)
                       .titlebar_visible(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_WINDOW_EXAMPLES_SERVICE_MATRIX") ==
      nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicWindowExamplesServiceMatrixView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

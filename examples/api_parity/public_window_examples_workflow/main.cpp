#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

struct InstallWorkflowMenuAction {
  static constexpr std::string_view name = "window_examples.workflow.menu";
};

struct FocusWorkflowInputAction {
  static constexpr std::string_view name = "window_examples.workflow.input";
};

struct PreviewWorkflowPositionAction {
  static constexpr std::string_view name = "window_examples.workflow.position";
};

struct ToggleWorkflowShadowAction {
  static constexpr std::string_view name = "window_examples.workflow.shadow";
};

class PublicWindowExamplesWorkflowView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicWindowExamplesWorkflowView>& context) override {
    const cgpui::TestContextCapability test = context.test_context();
    install_actions(context);
    install_menu_demo(context);
    drive_public_workflow(test);

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .bg(cgpui::rgb(18, 24, 30))
            .p(22.0F)
            .child(cgpui::v_flex()
                       .size_pct(100.0F, 100.0F)
                       .gap(12.0F)
                       .p(18.0F)
                       .rounded(8.0F)
                       .bg(cgpui::rgb(38, 47, 56))
                       .shadow(workflow_shadow())
                       .child(cgpui::label("window/examples public workflow")
                                  .font_size(19.0F)
                                  .foreground(cgpui::rgb(242, 246, 250))
                                  .build())
                       .child(cgpui::label("test-context driven menu, input, pointer, focus, and shadow flow")
                                  .font_size(12.0F)
                                  .foreground(cgpui::rgb(180, 193, 207))
                                  .build())
                       .child(cgpui::h_flex()
                                  .gap(10.0F)
                                  .child(cgpui::button(
                                             InstallWorkflowMenuAction::name)
                                             .label("Install workflow menu")
                                             .build())
                                  .child(cgpui::button(
                                             ToggleWorkflowShadowAction::name)
                                             .label("Toggle workflow shadow")
                                             .build()))
                       .child(cgpui::h_flex()
                                  .gap(10.0F)
                                  .child(cgpui::menu_item(
                                             PreviewWorkflowPositionAction::name)
                                             .label("Preview workflow position")
                                             .selected(true)
                                             .build())
                                  .child(cgpui::menu_item(
                                             FocusWorkflowInputAction::name)
                                             .label("Focus workflow input")
                                             .build()))
                       .child(cgpui::text_input(input_model_)
                                  .key("window-examples-workflow-input")
                                  .style(input_style())
                                  .foreground(cgpui::rgb(245, 248, 251))
                                  .font_size(14.0F)
                                  .build())
                       .child(cgpui::div()
                                  .fixed()
                                  .top(24.0F)
                                  .left(360.0F)
                                  .w(250.0F)
                                  .h(82.0F)
                                  .rounded(8.0F)
                                  .bg(cgpui::rgb(247, 250, 253))
                                  .shadow(workflow_shadow())
                                  .p(12.0F)
                                  .child(cgpui::label("Workflow shadow card")
                                             .font_size(13.0F)
                                             .foreground(cgpui::rgb(31, 40, 50))
                                             .build()))));
  }

 private:
  static cgpui::BoxShadow workflow_shadow() {
    return cgpui::BoxShadow{
        .color = cgpui::rgba(5, 10, 15, 0.30F),
        .offset = cgpui::Point{0.0F, 8.0F},
        .blur_radius = 22.0F,
        .spread_radius = 1.0F,
    };
  }

  static cgpui::Style input_style() {
    return cgpui::Style{}
        .with_preferred_size(cgpui::Size{450.0F, 42.0F})
        .with_background_color(cgpui::rgb(49, 60, 70))
        .with_border_width(cgpui::edges(1.0F))
        .with_border_color(cgpui::rgb(95, 113, 132))
        .with_border_radius(cgpui::BorderRadii::all(6.0F))
        .with_padding(cgpui::edges(12.0F, 8.0F));
  }

  void install_actions(
      cgpui::Context<PublicWindowExamplesWorkflowView>& context) const {
    context.register_command_palette_entry<InstallWorkflowMenuAction>(
        cgpui::CommandPaletteEntry{
            .title = "Install workflow menu",
            .group = "Window Examples Workflow",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-shift-m",
        });
    context.register_command_palette_entry<FocusWorkflowInputAction>(
        cgpui::CommandPaletteEntry{
            .title = "Focus workflow input",
            .group = "Window Examples Workflow",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-shift-i",
        });
    context.register_command_palette_entry<PreviewWorkflowPositionAction>(
        cgpui::CommandPaletteEntry{
            .title = "Preview workflow positioning",
            .group = "Window Examples Workflow",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-shift-p",
        });
    context.register_command_palette_entry<ToggleWorkflowShadowAction>(
        cgpui::CommandPaletteEntry{
            .title = "Toggle workflow shadow",
            .group = "Window Examples Workflow",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-shift-s",
        });

    (void)context.bind_key(
        "ctrl-shift-m",
        std::string(InstallWorkflowMenuAction::name),
        cgpui::KeyBindingContext::window());
    (void)context.bind_key(
        "ctrl-shift-i",
        std::string(FocusWorkflowInputAction::name),
        cgpui::KeyBindingContext::window());
  }

  void install_menu_demo(
      cgpui::Context<PublicWindowExamplesWorkflowView>& context) const {
    cgpui::KeyboardModifiers menu_modifiers{};
    menu_modifiers.control = true;
    menu_modifiers.shift = true;

    cgpui::NativeMenuAccelerator install_accelerator{};
    install_accelerator.key_code = 'M';
    install_accelerator.action = cgpui::KeyAction::pressed;
    install_accelerator.modifiers = menu_modifiers;

    cgpui::NativeMenuAccelerator focus_accelerator{};
    focus_accelerator.key_code = 'I';
    focus_accelerator.action = cgpui::KeyAction::pressed;
    focus_accelerator.modifiers = menu_modifiers;

    cgpui::NativeMenuItem install_item{};
    install_item.title = "Install Workflow Menu";
    install_item.action_name = std::string(InstallWorkflowMenuAction::name);
    install_item.accelerator = install_accelerator;

    cgpui::NativeMenuItem separator{};
    separator.kind = cgpui::NativeMenuItemKind::separator;

    cgpui::NativeMenuItem focus_item{};
    focus_item.title = "Focus Workflow Input";
    focus_item.action_name = std::string(FocusWorkflowInputAction::name);
    focus_item.accelerator = focus_accelerator;

    cgpui::NativeMenuItem submenu{};
    submenu.kind = cgpui::NativeMenuItemKind::submenu;
    submenu.title = "Workflow";
    submenu.children = {std::move(install_item), std::move(separator),
                        std::move(focus_item)};

    cgpui::NativeMenuModel menu{
        .items = {std::move(submenu)},
    };
    (void)context.try_install_native_menu(std::move(menu));
  }

  static void drive_public_workflow(const cgpui::TestContextCapability& test) {
    test.dispatch_window_activation(true);
    test.dispatch_window_focus(true);
    test.request_redraw();
    (void)test.try_draw_frame();
    (void)test.simulate_keystrokes("ctrl-shift-m");
    test.dispatch_pointer_move(cgpui::Point{36.0F, 138.0F});
    test.dispatch_pointer_button(
        cgpui::MouseButton::left,
        true,
        cgpui::Point{36.0F, 138.0F});
    test.dispatch_pointer_button(
        cgpui::MouseButton::left,
        false,
        cgpui::Point{36.0F, 138.0F});
  }

  cgpui::TextModel input_model_{"Type into the window/examples workflow"};
};

using WindowOptionsRef = cgpui::WindowOptions&;
using WindowWorkflowContextRef =
    cgpui::Context<PublicWindowExamplesWorkflowView>&;
using WindowWorkflowTestRef = cgpui::TestContextCapability&;

static_assert(cgpui::Action<InstallWorkflowMenuAction>);
static_assert(cgpui::Action<FocusWorkflowInputAction>);
static_assert(cgpui::Action<PreviewWorkflowPositionAction>);
static_assert(cgpui::Action<ToggleWorkflowShadowAction>);
static_assert(cgpui::Render<PublicWindowExamplesWorkflowView>);
static_assert(std::is_same_v<decltype(std::declval<WindowWorkflowContextRef>()
                                          .test_context()),
                             cgpui::TestContextCapability>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowWorkflowContextRef>()
                                .try_install_native_menu(
                                    std::declval<cgpui::NativeMenuModel>())),
                   cgpui::Result<cgpui::NativeMenuInstallation>>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowWorkflowContextRef>()
                                .bind_key(
                                    std::declval<std::string_view>(),
                                    std::declval<std::string>(),
                                    cgpui::KeyBindingContext::window())),
                   bool>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowWorkflowTestRef>()
                                .simulate_keystrokes(
                                    std::declval<std::string_view>())),
                   bool>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowWorkflowTestRef>()
                                .try_draw_frame()),
                   cgpui::Result<void>>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowOptionsRef>()
                                .titlebar_visible(true)),
                   cgpui::WindowOptions&>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Window Examples Workflow")
                       .size(700.0F, 460.0F)
                       .decorations(true)
                       .resizable(true)
                       .transparent(false)
                       .titlebar_visible(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_WINDOW_EXAMPLES_WORKFLOW") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicWindowExamplesWorkflowView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

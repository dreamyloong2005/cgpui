#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

struct ActivateInteractionStateAction {
  static constexpr std::string_view name =
      "window_examples.interaction.activate";
};

struct ToggleInteractionStateAction {
  static constexpr std::string_view name =
      "window_examples.interaction.toggle";
};

struct FocusInteractionInputAction {
  static constexpr std::string_view name =
      "window_examples.interaction.focus_input";
};

class PublicWindowExamplesInteractionStatesView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicWindowExamplesInteractionStatesView>& context)
      override {
    install_actions(context);

    const cgpui::StyleState state = interaction_state();
    const cgpui::StyleOverlay focus_overlay =
        cgpui::StyleOverlay{}
            .with_border_width(cgpui::edges(2.0F))
            .with_border_color(cgpui::rgb(136, 195, 255));

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .bg(cgpui::rgb(18, 23, 29))
            .p(22.0F)
            .child(cgpui::v_stack()
                       .size_pct(100.0F, 100.0F)
                       .gap(12.0F)
                       .p(18.0F)
                       .rounded(8.0F)
                       .bg(cgpui::rgb(37, 46, 56))
                       .shadow(interaction_shadow())
                       .child(cgpui::label(
                                  "window/examples interaction states")
                                  .font_size(19.0F)
                                  .foreground(cgpui::rgb(242, 246, 250))
                                  .build())
                       .child(cgpui::label(
                                  "hover/focus/active/disabled interaction states")
                                  .font_size(12.0F)
                                  .foreground(cgpui::rgb(181, 194, 207))
                                  .build())
                       .child(cgpui::h_stack()
                                  .gap(10.0F)
                                  .child(cgpui::button(
                                             ActivateInteractionStateAction::name)
                                             .label("Activate state")
                                             .style(state.base)
                                             .hover_style(state.hover)
                                             .focus_style(state.focus)
                                             .active_style(state.active)
                                             .disabled_style(state.disabled)
                                             .on_click([](
                                                 const cgpui::ElementEventContext&
                                                     event) {
                                               return event.dispatch_action(
                                                   ActivateInteractionStateAction::
                                                       name);
                                             })
                                             .key("interaction-primary-button")
                                             .build())
                                  .child(cgpui::button(
                                             ToggleInteractionStateAction::name)
                                             .label("Disabled example")
                                             .style(state.base)
                                             .disabled_style(state.disabled)
                                             .disabled()
                                             .key("interaction-disabled-button")
                                             .build()))
                       .child(cgpui::h_stack()
                                  .gap(10.0F)
                                  .child(cgpui::div()
                                             .style(focus_card_style())
                                             .focus_style(focus_overlay)
                                             .active_style(
                                                 cgpui::StyleOverlay{}
                                                     .with_background_color(
                                                         cgpui::rgb(50, 64, 77)))
                                             .focusable()
                                             .tab_index(1)
                                             .focus_ring(cgpui::FocusRingVisibility::visible)
                                             .on_click([](
                                                 const cgpui::ElementEventContext&) {
                                               return cgpui::EventResult::
                                                   consumed_event();
                                             })
                                             .child(cgpui::label(
                                                        "Focus ring and tab-index examples")
                                                        .font_size(13.0F)
                                                        .foreground(cgpui::rgb(
                                                            241, 246, 250))
                                                        .build()))
                                  .child(cgpui::menu_item(
                                             FocusInteractionInputAction::name)
                                             .label("Focus interaction input")
                                             .selected(true)
                                             .key("interaction-menu-item")
                                             .build()))
                       .child(cgpui::h_stack()
                                  .gap(10.0F)
                                  .child(cgpui::checkbox(
                                             ToggleInteractionStateAction::name)
                                             .label("Remember interaction")
                                             .checked(true)
                                             .key("interaction-checkbox")
                                             .build())
                                  .child(cgpui::text_input(input_model_)
                                             .key("interaction-input")
                                             .style(input_style())
                                             .foreground(
                                                 cgpui::rgb(245, 248, 251))
                                             .font_size(14.0F)
                                             .build()))));
  }

 private:
  void install_actions(
      cgpui::Context<PublicWindowExamplesInteractionStatesView>& context)
      const {
    context.register_command_palette_entry<ActivateInteractionStateAction>(
        cgpui::CommandPaletteEntry{
            .title = "Activate interaction state",
            .group = "Window Examples Interaction States",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-alt-a",
        });
    context.register_command_palette_entry<ToggleInteractionStateAction>(
        cgpui::CommandPaletteEntry{
            .title = "Toggle interaction state",
            .group = "Window Examples Interaction States",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-alt-t",
        });
    context.register_command_palette_entry<FocusInteractionInputAction>(
        cgpui::CommandPaletteEntry{
            .title = "Focus interaction input",
            .group = "Window Examples Interaction States",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-alt-i",
        });

    (void)context.bind_key(
        "ctrl-alt-a",
        std::string(ActivateInteractionStateAction::name),
        cgpui::KeyBindingContext::window());
    (void)context.bind_key(
        "ctrl-alt-i",
        std::string(FocusInteractionInputAction::name),
        cgpui::KeyBindingContext::window());
  }

  static cgpui::StyleState interaction_state() {
    return cgpui::StyleState{
        .base = cgpui::Style{}
                    .with_preferred_size(cgpui::Size{160.0F, 38.0F})
                    .with_background_color(cgpui::rgb(63, 90, 122))
                    .with_foreground_color(cgpui::rgb(244, 248, 252))
                    .with_border_width(cgpui::edges(1.0F))
                    .with_border_color(cgpui::rgb(91, 121, 153))
                    .with_border_radius(cgpui::BorderRadii::all(6.0F))
                    .with_padding(cgpui::edges(12.0F, 8.0F)),
        .hover = cgpui::StyleOverlay{}.with_background_color(
            cgpui::rgb(74, 105, 139)),
        .focus = cgpui::StyleOverlay{}
                     .with_border_width(cgpui::edges(2.0F))
                     .with_border_color(cgpui::rgb(139, 201, 255)),
        .active = cgpui::StyleOverlay{}
                      .with_background_color(cgpui::rgb(42, 67, 95))
                      .with_opacity(0.92F),
        .disabled = cgpui::StyleOverlay{}
                        .with_background_color(cgpui::rgb(55, 61, 68))
                        .with_foreground_color(cgpui::rgb(149, 158, 168))
                        .with_opacity(0.55F),
    };
  }

  static cgpui::Style focus_card_style() {
    return cgpui::Style{}
        .with_preferred_size(cgpui::Size{260.0F, 42.0F})
        .with_background_color(cgpui::rgb(44, 55, 66))
        .with_border_width(cgpui::edges(1.0F))
        .with_border_color(cgpui::rgb(82, 100, 119))
        .with_border_radius(cgpui::BorderRadii::all(6.0F))
        .with_padding(cgpui::edges(12.0F, 8.0F));
  }

  static cgpui::Style input_style() {
    return cgpui::Style{}
        .with_preferred_size(cgpui::Size{300.0F, 40.0F})
        .with_background_color(cgpui::rgb(49, 60, 70))
        .with_border_width(cgpui::edges(1.0F))
        .with_border_color(cgpui::rgb(95, 113, 132))
        .with_border_radius(cgpui::BorderRadii::all(6.0F))
        .with_padding(cgpui::edges(12.0F, 8.0F));
  }

  static cgpui::BoxShadow interaction_shadow() {
    return cgpui::BoxShadow{
        .color = cgpui::rgba(5, 10, 15, 0.30F),
        .offset = cgpui::Point{0.0F, 8.0F},
        .blur_radius = 22.0F,
        .spread_radius = 1.0F,
    };
  }

  cgpui::TextModel input_model_{"Interaction-state input example"};
};

using InteractionContextRef =
    cgpui::Context<PublicWindowExamplesInteractionStatesView>&;
using WindowOptionsRef = cgpui::WindowOptions&;

static_assert(cgpui::Action<ActivateInteractionStateAction>);
static_assert(cgpui::Action<ToggleInteractionStateAction>);
static_assert(cgpui::Action<FocusInteractionInputAction>);
static_assert(cgpui::Render<PublicWindowExamplesInteractionStatesView>);
static_assert(
    std::is_same_v<decltype(std::declval<InteractionContextRef>()
                                .bind_key(
                                    std::declval<std::string_view>(),
                                    std::declval<std::string>(),
                                    cgpui::KeyBindingContext::window())),
                   bool>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowOptionsRef>().resizable(true)),
                   cgpui::WindowOptions&>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Window Examples Interaction States")
                       .size(760.0F, 500.0F)
                       .decorations(true)
                       .resizable(true)
                       .transparent(false)
                       .titlebar_visible(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_WINDOW_EXAMPLES_INTERACTION_STATES") ==
      nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicWindowExamplesInteractionStatesView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

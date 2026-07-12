#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace {

struct RestartOpacityAnimationAction {
  static constexpr std::string_view name = "opacity.restart_animation";
};

class PublicOpacityExampleView final : public cgpui::View {
 public:
  cgpui::IntoElement render(
      cgpui::Context<PublicOpacityExampleView>& context) override {
    context.register_action(
        std::string(RestartOpacityAnimationAction::name),
        [this](const cgpui::ViewContext& action_context) {
          if (transition_.active()) (void)transition_.cancel();
          opacity_ = 0.0F;
          transition_ = cgpui::start_animation_transition(
              action_context,
              cgpui::AnimationTransition{.from = 0.0F, .to = 1.0F},
              cgpui::AnimationOptions{
                  .duration_ms = 1'600,
                  .easing = cgpui::AnimationEasing::linear,
                  .tick_interval_ms = 16,
              },
              [this](
                  const cgpui::ViewContext& animation_context,
                  const cgpui::AnimationTransitionSnapshot& snapshot) {
                opacity_ = snapshot.value;
                animation_context.request_render();
              });
          action_context.request_render();
          return cgpui::EventResult::consumed_event();
        });

    auto panel = cgpui::div()
                     .absolute()
                     .inset(cgpui::edges(38.0F))
                     .opacity(opacity_)
                     .bg(cgpui::rgb(247, 249, 251))
                     .border_width(cgpui::edges(3.0F))
                     .border_color(cgpui::rgb(198, 54, 54))
                     .rounded(10.0F)
                     .shadow(cgpui::BoxShadow{
                         .color = cgpui::rgba(0, 0, 0, 0.35F),
                         .offset = {10.0F, 10.0F},
                         .blur_radius = 12.0F,
                         .spread_radius = 5.0F,
                     })
                     .on_click([](const cgpui::ElementEventContext& event) {
                       return event.dispatch_action(
                           RestartOpacityAnimationAction::name);
                     })
                     .child(cgpui::v_stack()
                                .size_pct(100.0F, 100.0F)
                                .items_center()
                                .justify_center()
                                .gap(12.0F)
                                .child(cgpui::label("Opacity Panel")
                                           .font_size(25.0F)
                                           .foreground(cgpui::rgb(24, 65, 118))
                                           .build())
                                .child(cgpui::label(
                                           "Click the panel to restart")
                                           .font_size(14.0F)
                                           .foreground(cgpui::rgb(66, 76, 86))
                                           .build()));

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .relative()
            .bg(cgpui::rgb(224, 224, 224))
            .child(cgpui::v_stack()
                       .size_pct(100.0F, 100.0F)
                       .items_center()
                       .justify_center()
                       .child(cgpui::label("This is background text.")
                                  .font_size(22.0F)
                                  .foreground(cgpui::rgb(35, 88, 164))
                                  .build()))
            .child(std::move(panel)));
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {}

 private:
  float opacity_ = 0.5F;
  cgpui::AnimationTransitionHandle transition_;
};

static_assert(cgpui::Action<RestartOpacityAnimationAction>);
static_assert(cgpui::Render<PublicOpacityExampleView>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Opacity Example")
                       .size(500.0F, 500.0F)
                       .resizable(true)
                       .to_descriptor();
  if (std::getenv("CGPUI_RUN_PUBLIC_OPACITY_EXAMPLE") == nullptr) return 0;

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }
  PublicOpacityExampleView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

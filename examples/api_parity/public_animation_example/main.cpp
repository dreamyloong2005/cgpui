#include "cgpui/prelude.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numbers>
#include <type_traits>
#include <utility>

namespace {

cgpui::AffineTransform rotation_transform(float turns) {
  const float radians = turns * 2.0F * std::numbers::pi_v<float>;
  const float sine = std::sin(radians);
  const float cosine = std::cos(radians);
  return cgpui::AffineTransform{
      .scale_x = cosine,
      .skew_y = sine,
      .skew_x = -sine,
      .scale_y = cosine,
  };
}

class PublicAnimationExampleView final : public cgpui::View {
 public:
  cgpui::IntoElement render(
      cgpui::Context<PublicAnimationExampleView>& context) override {
    auto animated_badge = cgpui::with_animations(
        cgpui::ElementKey{.value = "official-animation-badge"},
        {cgpui::ElementAnimationStage{
             .animation = cgpui::AnimationOptions{
                 .duration_ms = 2'000,
                 .curve = cgpui::AnimationCurve::bounce(
                     cgpui::AnimationEasing::ease_in_out),
                 .tick_interval_ms = 16,
             },
         }.repeat()},
        cgpui::into_element(
            cgpui::div()
                .size(96.0F, 96.0F)
                .rounded(18.0F)
                .bg(cgpui::rgb(41, 102, 168))
                .border_1()
                .border_color(cgpui::rgb(111, 181, 246))
                .shadow(cgpui::BoxShadow{
                    .color = cgpui::rgba(7, 26, 45, 0.35F),
                    .offset = {0.0F, 10.0F},
                    .blur_radius = 24.0F,
                    .spread_radius = 1.0F,
                })
                .child(cgpui::label("Rotate")
                           .font_size(17.0F)
                           .foreground(cgpui::rgb(245, 250, 255))
                           .build())),
        [](cgpui::Element& element,
           const cgpui::ElementAnimationSnapshot& snapshot) {
          auto* styled = dynamic_cast<cgpui::StyledElement*>(&element);
          if (styled == nullptr) return;
          styled->set_inline_style(
              cgpui::StyleOverlay{}.with_transform(
                  rotation_transform(snapshot.eased_progress)));
        });

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .bg(cgpui::rgb(19, 23, 28))
            .p(24.0F)
            .child(cgpui::v_stack()
                       .size_pct(100.0F, 100.0F)
                       .items_center()
                       .justify_center()
                       .gap(22.0F)
                       .child(cgpui::label("Hello Animation")
                                  .font_size(26.0F)
                                  .foreground(cgpui::rgb(244, 247, 250))
                                  .build())
                       .child(std::move(animated_badge))
                       .child(cgpui::label("Other Panel")
                                  .font_size(14.0F)
                                  .foreground(cgpui::rgb(170, 181, 191))
                                  .build())));
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

static_assert(cgpui::Render<PublicAnimationExampleView>);
static_assert(std::is_same_v<
              decltype(rotation_transform(0.5F)),
              cgpui::AffineTransform>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Animation Example")
                       .size(420.0F, 420.0F)
                       .resizable(true)
                       .to_descriptor();
  if (std::getenv("CGPUI_RUN_PUBLIC_ANIMATION_EXAMPLE") == nullptr) return 0;

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }
  PublicAnimationExampleView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

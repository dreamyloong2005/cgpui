#include "window_runtime_test_support.hpp"

namespace {

bool equal_color(cgpui::Color lhs, cgpui::Color rhs) {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

int test_window_context_theme_helpers_override_and_fallback() {
  RuntimeFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::Theme app_theme;
  app_theme.set_color(
      cgpui::theme_token("accent"),
      cgpui::Color{.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 1.0F});
  app_theme.set_spacing(cgpui::theme_token("gap"), 4.0F);
  runtime.set_app_theme(std::move(app_theme));

  cgpui::WindowRuntimeContext context{
      .runtime = runtime,
      .application = fixture.app,
      .window = fixture.window,
      .renderer = fixture.renderer,
      .window_runtime_id = runtime.root_window_runtime_id(),
      .view_id = cgpui::ViewId{1},
      .viewport_size = {.width = 640.0F, .height = 480.0F},
      .scale = {.value = 1.0F},
  };

  const std::optional<cgpui::Color> app_accent =
      context.theme_color(cgpui::theme_token("accent"));
  const std::optional<float> app_gap =
      context.theme_spacing(cgpui::theme_token("gap"));
  if (!app_accent.has_value() ||
      !equal_color(*app_accent,
                   cgpui::Color{.r = 0.1F,
                                .g = 0.2F,
                                .b = 0.3F,
                                .a = 1.0F}) ||
      !app_gap.has_value() || *app_gap != 4.0F ||
      context.window_theme() != nullptr) {
    return 1;
  }

  cgpui::Theme window_theme;
  window_theme.set_color(
      cgpui::theme_token("accent"),
      cgpui::Color{.r = 0.7F, .g = 0.6F, .b = 0.5F, .a = 1.0F});
  context.set_window_theme(std::move(window_theme));

  const std::optional<cgpui::Color> window_accent =
      context.theme_color(cgpui::theme_token("accent"));
  const std::optional<float> fallback_gap =
      context.theme_spacing(cgpui::theme_token("gap"));
  if (context.window_theme() == nullptr || !window_accent.has_value() ||
      !equal_color(*window_accent,
                   cgpui::Color{.r = 0.7F,
                                .g = 0.6F,
                                .b = 0.5F,
                                .a = 1.0F}) ||
      !fallback_gap.has_value() || *fallback_gap != 4.0F) {
    return 2;
  }

  if (!context.clear_window_theme() || context.window_theme() != nullptr) {
    return 3;
  }
  const std::optional<cgpui::Color> restored_accent =
      context.theme_color(cgpui::theme_token("accent"));
  if (!restored_accent.has_value() ||
      !equal_color(*restored_accent,
                   cgpui::Color{.r = 0.1F,
                                .g = 0.2F,
                                .b = 0.3F,
                                .a = 1.0F})) {
    return 4;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_window_context_theme_helpers_override_and_fallback();
      result != 0) {
    return result;
  }
  return 0;
}

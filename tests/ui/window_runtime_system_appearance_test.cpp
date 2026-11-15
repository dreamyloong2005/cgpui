#include "cgpui/app/app.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/ui.hpp"

#include <algorithm>
#include <expected>
#include <iostream>
#include <memory>
#include <vector>

namespace {

class BareApplication : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&, cgpui::PlatformEventCallback) override {
    return std::unexpected(cgpui::Error{
        .code = cgpui::ErrorCode::unsupported_platform,
        .message = "appearance test creates no windows"});
  }
  int run() override { return 0; }
  void quit() override {}
};

class AppearanceApplication final : public BareApplication {
 public:
  cgpui::SystemAppearance system_appearance() const override {
    return appearance;
  }
  void set_system_appearance_callback(
      cgpui::SystemAppearanceCallback next) override {
    callback = std::move(next);
    install_count += 1;
  }
  void emit(bool changed) {
    if (callback) {
      callback(cgpui::SystemAppearanceChange{
          .source = cgpui::SystemAppearanceChangeSource::color_set,
          .appearance = appearance,
          .changed = changed});
    }
  }

  cgpui::SystemAppearance appearance{
      .supported = true,
      .backend = "fake",
      .mode = cgpui::SystemAppearanceMode::dark,
      .accent_color_available = true,
      .accent_color = cgpui::Color{0.0F, 0.47F, 0.84F, 1.0F},
      .transparency_effects_enabled = true,
      .client_area_animation_enabled = false,
      .high_contrast = false};
  cgpui::SystemAppearanceCallback callback;
  int install_count = 0;
};

class EmptyView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

cgpui::RendererFactory no_renderer() {
  return [](const cgpui::RenderSurfaceDescriptor&)
             -> cgpui::Result<cgpui::Renderer*> {
    return std::unexpected(cgpui::Error{
        .code = cgpui::ErrorCode::unsupported_platform, .message = "none"});
  };
}

int fail(int code, const char* message) {
  std::cerr << "runtime system appearance: " << message << '\n';
  return code;
}

int check_fallback() {
  BareApplication bare;
  const cgpui::SystemAppearance appearance = bare.system_appearance();
  if (appearance.supported || appearance.backend != "unsupported" ||
      appearance.mode != cgpui::SystemAppearanceMode::light ||
      appearance.accent_color_available || appearance.high_contrast) {
    return fail(1, "fallback appearance fabricated a native reading");
  }
  const cgpui::PlatformDpiAwarenessState dpi = bare.dpi_awareness();
  if (dpi.awareness != cgpui::PlatformDpiAwareness::unsupported ||
      dpi.declared_by_framework || dpi.backend != "unsupported") {
    return fail(2, "fallback dpi awareness fabricated a declaration");
  }
  bare.set_system_appearance_callback(
      [](const cgpui::SystemAppearanceChange&) {});
  return 0;
}

} // namespace

int main() {
  if (const int code = check_fallback(); code != 0) return code;

  AppearanceApplication platform;
  EmptyView view;
  std::vector<cgpui::SystemAppearanceChange> seen;
  {
    cgpui::WindowRuntime runtime(platform, view, no_renderer());
    const cgpui::App app(runtime);
    if (!(app.system_appearance() == platform.appearance) ||
        !(app.root_window().appearance() == platform.appearance)) {
      return fail(3, "App/Window queries do not forward the platform snapshot");
    }
    app.set_system_appearance_callback(
        [&](const cgpui::SystemAppearanceChange& change) {
          seen.push_back(change);
        });
    app.set_system_appearance_callback(
        [&](const cgpui::SystemAppearanceChange& change) {
          seen.push_back(change);
          seen.push_back(change);
        });
    if (platform.install_count != 1 || !platform.callback) {
      return fail(4, "runtime must install exactly one platform bridge");
    }
    runtime.clear_invalidation();
    platform.emit(false);
    if (seen.size() != 2 || runtime.invalidation_state().render) {
      return fail(5, "unchanged notification must reach the latest callback only");
    }
    platform.emit(true);
    if (seen.size() != 4 || !seen.back().changed ||
        !runtime.invalidation_state().render) {
      return fail(6, "changed notification must request a render");
    }
    const auto diagnostics = runtime.platform_diagnostics();
    const auto count = std::ranges::count_if(
        diagnostics, [](const cgpui::PlatformDiagnosticEvent& event) {
          return event.kind == cgpui::PlatformDiagnosticKind::appearance &&
                 event.operation == "system-appearance-changed" &&
                 event.backend == "fake";
        });
    if (count != 2) return fail(7, "appearance diagnostics not recorded");
  }
  if (platform.callback || platform.install_count != 2) {
    return fail(8, "runtime destruction must clear the platform bridge");
  }
  return 0;
}

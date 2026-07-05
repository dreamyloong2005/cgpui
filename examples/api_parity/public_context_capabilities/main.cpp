#include "cgpui/prelude.hpp"

#include <concepts>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace {

struct CapabilityGlobalState {
  int revision = 0;
};

struct CapabilityElementState {
  int focus_count = 0;
};

class PublicContextCapabilitiesView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicContextCapabilitiesView>& context) override {
    const cgpui::AppContext app_context = context.app_context();
    const cgpui::ViewContextCapability<PublicContextCapabilitiesView>
        view_context =
            context.view_context<PublicContextCapabilitiesView>();
    const cgpui::WindowContextCapability window_context =
        context.window_context();
    const cgpui::ElementId root_element{9};
    const cgpui::ElementContextCapability element_context =
        context.element_context(root_element);

    app_context.set_global(CapabilityGlobalState{.revision = 1});
    (void)app_context.update_global<CapabilityGlobalState>(
        [](CapabilityGlobalState& state) {
          state.revision += 1;
        });

    cgpui::Theme theme;
    theme.set_color(cgpui::ThemeTokenId{"capability.accent"},
                    cgpui::rgb(74, 135, 196));
    app_context.set_app_theme(theme);

    const cgpui::ViewHandle<PublicContextCapabilitiesView> view =
        view_context.view();
    const cgpui::WeakViewHandle<PublicContextCapabilitiesView> weak_view =
        view_context.weak_view();
    const cgpui::Window window = window_context.window();

    window_context.request_render();
    window_context.request_layout();
    window_context.request_paint();

    element_context.focus();
    element_context.capture_pointer();
    element_context.set_cursor(cgpui::CursorShape::pointing_hand);
    (void)element_context.state_or_init<CapabilityElementState>(
        CapabilityElementState{.focus_count = 1});
    element_context.release_pointer();
    element_context.blur();

    (void)view;
    (void)weak_view;
    (void)window;

    return cgpui::into_element(
        cgpui::v_flex()
            .size(context.viewport_size)
            .padding(cgpui::edges(18.0F))
            .gap(8.0F)
            .background(cgpui::rgb(22, 31, 38))
            .child(cgpui::label("Public context capabilities")
                       .font_size(18.0F)
                       .foreground(cgpui::rgb(238, 243, 246))
                       .build())
            .child(cgpui::label("App, view, window, and element surfaces")
                       .font_size(13.0F)
                       .foreground(cgpui::rgb(180, 196, 206))
                       .build()));
  }
};

using ContextRef = cgpui::Context<PublicContextCapabilitiesView>&;
using ViewCapability =
    cgpui::ViewContextCapability<PublicContextCapabilitiesView>;

static_assert(cgpui::Render<PublicContextCapabilitiesView>);
static_assert(std::same_as<decltype(std::declval<ContextRef>().app_context()),
                           cgpui::AppContext>);
static_assert(std::same_as<decltype(std::declval<ContextRef>()
                                        .view_context<PublicContextCapabilitiesView>()),
                           ViewCapability>);
static_assert(std::same_as<decltype(std::declval<ContextRef>()
                                        .window_context()),
                           cgpui::WindowContextCapability>);
static_assert(std::same_as<decltype(std::declval<ContextRef>()
                                        .element_context(
                                            std::declval<cgpui::ElementId>())),
                           cgpui::ElementContextCapability>);
static_assert(std::same_as<decltype(std::declval<cgpui::AppContext>()
                                        .set_global(
                                            std::declval<CapabilityGlobalState>())),
                           void>);
static_assert(std::same_as<decltype(std::declval<ViewCapability>().view()),
                           cgpui::ViewHandle<PublicContextCapabilitiesView>>);
static_assert(std::same_as<decltype(std::declval<ViewCapability>().weak_view()),
                           cgpui::WeakViewHandle<PublicContextCapabilitiesView>>);
static_assert(std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                                        .window()),
                           cgpui::Window>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::ElementContextCapability>()
                              .state_or_init<CapabilityElementState>(
                                  std::declval<CapabilityElementState>())),
                 CapabilityElementState*>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Public Context Capabilities")
                       .size(470.0F, 260.0F)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_CONTEXT_CAPABILITIES") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicContextCapabilitiesView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

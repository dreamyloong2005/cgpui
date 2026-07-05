#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

struct RefreshPhaseBAction {
  static constexpr std::string_view name = "phase_b.refresh";
};

struct ClosePhaseBAction {
  static constexpr std::string_view name = "phase_b.close";
};

struct PhaseBState {
  std::string title;
  int revision = 0;
};

struct PhaseBGlobalState {
  int launch_count = 0;
};

struct PhaseBElementState {
  bool focused = false;
};

class PublicPhaseBSurfaceClosureView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicPhaseBSurfaceClosureView>& context) override {
    const cgpui::App app = context.app();
    const cgpui::Window window = context.window();
    const cgpui::AppContext app_context = context.app_context();
    const cgpui::ViewContextCapability<PublicPhaseBSurfaceClosureView>
        view_context =
            context.view_context<PublicPhaseBSurfaceClosureView>();
    const cgpui::WindowContextCapability window_context =
        context.window_context();
    const cgpui::ElementId root_element{13};
    const cgpui::ElementContextCapability element_context =
        context.element_context(root_element);
    const cgpui::AsyncContextCapability async = context.async_context();
    const cgpui::TestContextCapability test = context.test_context();

    app_context.set_global(PhaseBGlobalState{.launch_count = 1});
    (void)app_context.update_global<PhaseBGlobalState>(
        [](PhaseBGlobalState& state) {
          state.launch_count += 1;
        });

    const cgpui::EntityHandle<PhaseBState> state =
        context.new_entity<PhaseBState>("Phase B surface closure", 312);
    const cgpui::WeakEntity<PhaseBState> weak_state = state.downgrade();
    const cgpui::ViewHandle<PublicPhaseBSurfaceClosureView> view =
        view_context.view();
    const cgpui::WeakViewHandle<PublicPhaseBSurfaceClosureView> weak_view =
        view_context.weak_view();

    context.register_action(
        std::string(RefreshPhaseBAction::name),
        [](const cgpui::ViewContext& action_context) {
          action_context.request_render();
          return cgpui::EventResult::consumed_event();
        });
    context.register_command_palette_entry<RefreshPhaseBAction>(
        cgpui::CommandPaletteEntry{
            .title = "Refresh Phase B surface",
            .group = "Phase B",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-shift-r",
        });
    (void)context.bind_key(
        "ctrl-shift-r",
        std::string(RefreshPhaseBAction::name),
        cgpui::KeyBindingContext::focused_element(root_element));

    cgpui::NativeMenuModel menu{
        .items =
            {
                cgpui::NativeMenuItem{
                    .title = "Refresh",
                    .action_name = std::string(RefreshPhaseBAction::name),
                },
                cgpui::NativeMenuItem{
                    .title = "Close",
                    .action_name = std::string(ClosePhaseBAction::name),
                    .enabled = false,
                },
            },
    };
    const cgpui::Result<cgpui::NativeMenuInstallation> menu_result =
        context.try_install_native_menu(std::move(menu));
    const cgpui::Result<cgpui::NativeFileDialogResult> dialog_result =
        context.try_show_native_file_dialog(cgpui::NativeFileDialogOptions{
            .kind = cgpui::NativeFileDialogKind::open_file,
            .title = "Open Phase B Project",
        });

    const cgpui::Result<cgpui::TaskHandle> foreground =
        async.try_spawn_task([](const auto& completion) {
          completion.request_render();
        });
    const cgpui::Result<cgpui::TaskHandle> background =
        async.try_spawn_background_task(
            [](cgpui::TaskCancellationToken token) {
              (void)token.cancellation_requested();
            },
            [](const auto& completion) {
              completion.request_paint();
            });

    element_context.focus();
    element_context.capture_pointer();
    (void)element_context.state_or_init<PhaseBElementState>(
        PhaseBElementState{.focused = true});
    element_context.release_pointer();

    window_context.request_render();
    test.request_redraw();
    (void)test.try_draw_frame();

    (void)app;
    (void)window;
    (void)weak_state;
    (void)view;
    (void)weak_view;
    (void)menu_result;
    (void)dialog_result;
    (void)foreground;
    (void)background;

    return cgpui::into_element(
        cgpui::v_flex()
            .size(context.viewport_size)
            .padding(cgpui::edges(18.0F))
            .gap(8.0F)
            .background(cgpui::rgb(25, 31, 37))
            .child(cgpui::label("Phase B public surface")
                       .font_size(18.0F)
                       .foreground(cgpui::rgb(239, 244, 249))
                       .build())
            .child(cgpui::label("Prelude-only closure example")
                       .font_size(13.0F)
                       .foreground(cgpui::rgb(181, 193, 207))
                       .build()));
  }
};

using AppRef = cgpui::App&;
using AppContextRef = cgpui::AppContext&;
using ContextRef = cgpui::Context<PublicPhaseBSurfaceClosureView>&;
using AsyncRef = cgpui::AsyncContextCapability&;
using TestRef = cgpui::TestContextCapability&;
using ViewCapability =
    cgpui::ViewContextCapability<PublicPhaseBSurfaceClosureView>;

[[maybe_unused]] constexpr cgpui::ErrorCode kClosureErrorCode =
    cgpui::ErrorCode::invalid_argument;

static_assert(cgpui::Action<RefreshPhaseBAction>);
static_assert(cgpui::Action<ClosePhaseBAction>);
static_assert(cgpui::Render<PublicPhaseBSurfaceClosureView>);
static_assert(std::is_same_v<decltype(std::declval<AppRef>().try_open_window(
                                          std::declval<cgpui::WindowOptions>())),
                             cgpui::Result<cgpui::AppOpenedWindow>>);
static_assert(
    std::is_same_v<decltype(std::declval<AppContextRef>()
                                .try_install_native_menu(
                                    std::declval<cgpui::NativeMenuModel>())),
                   cgpui::Result<cgpui::NativeMenuInstallation>>);
static_assert(
    std::is_same_v<decltype(std::declval<ContextRef>()
                                .try_show_native_file_dialog(
                                    std::declval<cgpui::NativeFileDialogOptions>())),
                   cgpui::Result<cgpui::NativeFileDialogResult>>);
static_assert(std::is_same_v<decltype(std::declval<ContextRef>().app_context()),
                             cgpui::AppContext>);
static_assert(std::is_same_v<decltype(std::declval<ContextRef>()
                                          .view_context<
                                              PublicPhaseBSurfaceClosureView>()),
                             ViewCapability>);
static_assert(std::is_same_v<decltype(std::declval<ContextRef>()
                                          .window_context()),
                             cgpui::WindowContextCapability>);
static_assert(std::is_same_v<decltype(std::declval<ContextRef>()
                                          .element_context(
                                              std::declval<cgpui::ElementId>())),
                             cgpui::ElementContextCapability>);
static_assert(std::is_same_v<decltype(std::declval<AsyncRef>().try_spawn_task(
                                          std::declval<cgpui::TaskCompletionCallback>())),
                             cgpui::Result<cgpui::TaskHandle>>);
static_assert(
    std::is_same_v<decltype(std::declval<AsyncRef>().try_spawn_background_task(
                       std::declval<cgpui::BackgroundTaskCallback>(),
                       std::declval<cgpui::TaskCompletionCallback>())),
                   cgpui::Result<cgpui::TaskHandle>>);
static_assert(std::is_same_v<decltype(std::declval<TestRef>().try_draw_frame()),
                             cgpui::Result<void>>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Phase B Public Surface")
                       .size(490.0F, 270.0F)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_PHASE_B_SURFACE_CLOSURE") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicPhaseBSurfaceClosureView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

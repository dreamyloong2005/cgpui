#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

struct OpenProjectAction {
  static constexpr std::string_view name = "workflow.open_project";
};

struct SaveProjectAction {
  static constexpr std::string_view name = "workflow.save_project";
};

struct WorkflowState {
  std::string title;
  int revision = 0;
};

class PublicAuthoringWorkflowView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicAuthoringWorkflowView>& context) override {
    const cgpui::App app = context.app();
    const cgpui::Window window = context.window();
    const cgpui::ViewHandle<PublicAuthoringWorkflowView> view =
        context.view<PublicAuthoringWorkflowView>();
    const cgpui::WeakViewHandle<PublicAuthoringWorkflowView> weak_view =
        view.downgrade();
    const cgpui::EntityHandle<WorkflowState> state =
        context.new_entity<WorkflowState>("Workflow", 1);
    const cgpui::WeakEntity<WorkflowState> weak_state = state.downgrade();
    const cgpui::ElementId command_target{7};

    context.register_action(
        std::string(OpenProjectAction::name),
        [](const cgpui::ViewContext& action_context) {
          action_context.request_render();
          return cgpui::EventResult::consumed_event();
        });
    context.register_command_palette_entry<OpenProjectAction>(
        cgpui::CommandPaletteEntry{
            .title = "Open Project",
            .group = "Workflow",
            .scope = cgpui::ActionScope::view,
            .key_binding = "ctrl-o",
        });
    (void)context.bind_key(
        "ctrl-o",
        std::string(OpenProjectAction::name),
        cgpui::KeyBindingContext::focused_element(command_target));

    cgpui::NativeMenuModel menu{
        .items =
            {
                cgpui::NativeMenuItem{
                    .title = "Open Project",
                    .action_name = std::string(OpenProjectAction::name),
                },
                cgpui::NativeMenuItem{
                    .title = "Save Project",
                    .action_name = std::string(SaveProjectAction::name),
                    .enabled = false,
                },
            },
    };
    (void)context.try_install_native_menu(std::move(menu));
    (void)context.try_show_native_file_dialog(cgpui::NativeFileDialogOptions{
        .kind = cgpui::NativeFileDialogKind::open_file,
        .title = "Open Project",
        .filters =
            {
                cgpui::NativeFileDialogFilter{
                    .name = "Project",
                    .extensions = {"json", "toml"},
                },
            },
    });

    const cgpui::AsyncContextCapability async = context.async_context();
    const cgpui::TestContextCapability test = context.test_context();
    (void)async.try_spawn_task([](const cgpui::ViewContext&) {});

    (void)app;
    (void)window;
    (void)weak_view;
    (void)weak_state;
    (void)test;

    return cgpui::into_element(
        cgpui::v_flex()
            .size(context.viewport_size)
            .padding(cgpui::edges(18.0F))
            .gap(8.0F)
            .background(cgpui::rgb(24, 29, 35))
            .child(cgpui::label("Public authoring workflow")
                       .font_size(18.0F)
                       .foreground(cgpui::rgb(238, 242, 247))
                       .build())
            .child(cgpui::label("Prelude-only API coverage")
                       .font_size(13.0F)
                       .foreground(cgpui::rgb(178, 188, 201))
                       .build()));
  }
};

using AppRef = cgpui::App&;
using AppContextRef = cgpui::AppContext&;
using WorkflowContextRef = cgpui::Context<PublicAuthoringWorkflowView>&;
using AsyncRef = cgpui::AsyncContextCapability&;
using TestRef = cgpui::TestContextCapability&;

static_assert(cgpui::Action<OpenProjectAction>);
static_assert(cgpui::Action<SaveProjectAction>);
static_assert(cgpui::Render<PublicAuthoringWorkflowView>);
static_assert(std::is_same_v<
              decltype(std::declval<AppRef>().try_open_window(
                  std::declval<cgpui::WindowOptions>())),
              cgpui::Result<cgpui::AppOpenedWindow>>);
static_assert(std::is_same_v<
              decltype(std::declval<AppContextRef>().try_install_native_menu(
                  std::declval<cgpui::NativeMenuModel>())),
              cgpui::Result<cgpui::NativeMenuInstallation>>);
static_assert(std::is_same_v<
              decltype(std::declval<WorkflowContextRef>()
                           .try_show_native_file_dialog(
                               std::declval<cgpui::NativeFileDialogOptions>())),
              cgpui::Result<cgpui::NativeFileDialogResult>>);
static_assert(std::is_same_v<
              decltype(std::declval<AsyncRef>().try_spawn_task(
                  std::declval<cgpui::TaskCompletionCallback>())),
              cgpui::Result<cgpui::TaskHandle>>);
static_assert(std::is_same_v<
              decltype(std::declval<TestRef>().try_draw_frame()),
              cgpui::Result<void>>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Public Authoring Workflow")
                       .size(460.0F, 260.0F)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_AUTHORING_WORKFLOW") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicAuthoringWorkflowView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

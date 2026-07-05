#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

class PublicAsyncTestWorkflowView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicAsyncTestWorkflowView>& context) override {
    const cgpui::AsyncContextCapability async = context.async_context();
    const cgpui::TestContextCapability test = context.test_context();
    const cgpui::ElementId focus_target{11};

    async.defer([](const auto& deferred) {
      deferred.request_render();
    });
    const cgpui::TimerId wakeup = async.schedule_timer(
        1,
        [](const auto& timer) {
          timer.request_layout();
        });
    const cgpui::TimerId repeating = async.schedule_repeating_timer(
        16,
        [](const auto& timer) {
          timer.request_paint();
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
    async.batch_updates([](const auto& batch) {
      batch.request_layout();
    });

    test.run_until_parked();
    test.advance_time_until_parked(1);
    test.request_redraw();
    (void)test.try_draw_frame();
    test.draw_frame();
    test.dispatch_keystroke(cgpui::KeyboardKey{
        .key_code = 'A',
        .action = cgpui::KeyAction::pressed,
    });
    (void)test.simulate_keystrokes("ctrl-a");
    test.dispatch_pointer_move(cgpui::Point{18.0F, 22.0F});
    test.dispatch_pointer_button(
        cgpui::MouseButton::left,
        true,
        cgpui::Point{18.0F, 22.0F});
    test.dispatch_pointer_scroll(
        cgpui::Point{0.0F, -4.0F},
        cgpui::Point{18.0F, 22.0F});
    test.dispatch_window_activation(true);
    test.dispatch_window_focus(true);
    test.focus(focus_target);
    test.release_focus(focus_target);
    (void)test.write_to_clipboard("public async test workflow");
    const std::optional<std::string> clipboard_text =
        test.read_from_clipboard();
    (void)test.paste_clipboard_text();

    (void)wakeup;
    (void)repeating;
    (void)foreground;
    (void)background;
    (void)clipboard_text;

    return cgpui::into_element(
        cgpui::v_flex()
            .size(context.viewport_size)
            .padding(cgpui::edges(18.0F))
            .gap(8.0F)
            .background(cgpui::rgb(27, 30, 36))
            .child(cgpui::label("Public async and test workflow")
                       .font_size(18.0F)
                       .foreground(cgpui::rgb(239, 242, 248))
                       .build())
            .child(cgpui::label("Prelude-only scheduling and test helpers")
                       .font_size(13.0F)
                       .foreground(cgpui::rgb(184, 193, 205))
                       .build()));
  }
};

using ContextRef = cgpui::Context<PublicAsyncTestWorkflowView>&;
using AsyncRef = cgpui::AsyncContextCapability&;
using TestRef = cgpui::TestContextCapability&;

static_assert(cgpui::Render<PublicAsyncTestWorkflowView>);
static_assert(std::is_same_v<decltype(std::declval<ContextRef>()
                                          .async_context()),
                             cgpui::AsyncContextCapability>);
static_assert(std::is_same_v<decltype(std::declval<ContextRef>()
                                          .test_context()),
                             cgpui::TestContextCapability>);
static_assert(std::is_same_v<
              decltype(std::declval<AsyncRef>().try_spawn_task(
                  std::declval<cgpui::TaskCompletionCallback>())),
              cgpui::Result<cgpui::TaskHandle>>);
static_assert(
    std::is_same_v<
        decltype(std::declval<AsyncRef>().try_spawn_background_task(
            std::declval<cgpui::BackgroundTaskCallback>(),
            std::declval<cgpui::TaskCompletionCallback>())),
        cgpui::Result<cgpui::TaskHandle>>);
static_assert(std::is_same_v<decltype(std::declval<TestRef>()
                                          .run_until_parked()),
                             void>);
static_assert(
    std::is_same_v<decltype(std::declval<TestRef>()
                                .advance_time_until_parked(std::uint64_t{})),
                   void>);
static_assert(std::is_same_v<decltype(std::declval<TestRef>()
                                          .try_draw_frame()),
                             cgpui::Result<void>>);
static_assert(std::is_same_v<decltype(std::declval<TestRef>()
                                          .simulate_keystrokes(
                                              std::declval<std::string_view>())),
                             bool>);
static_assert(std::is_same_v<decltype(std::declval<TestRef>()
                                          .write_to_clipboard(
                                              std::declval<std::string_view>())),
                             bool>);
static_assert(std::is_same_v<decltype(std::declval<TestRef>()
                                          .read_from_clipboard()),
                             std::optional<std::string>>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Public Async Test Workflow")
                       .size(480.0F, 260.0F)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_ASYNC_TEST_WORKFLOW") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicAsyncTestWorkflowView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}

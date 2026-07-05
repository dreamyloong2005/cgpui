#include "cgpui/prelude.hpp"

#include <concepts>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

class TestContextCapabilityView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<TestContextCapabilityView>& context) override {
    const cgpui::TestContextCapability test_context =
        context.test_context();

    const cgpui::WindowRuntimeId runtime_id = test_context.runtime_id();
    const cgpui::ViewId view_id = test_context.view_id();
    const int frame_index = test_context.frame_index();
    const cgpui::ViewInputState input = test_context.input_state();
    const std::optional<cgpui::EventRoute> route =
        test_context.current_event_route();
    const std::optional<cgpui::ActionDispatchResult> action =
        test_context.last_action_dispatch();
    const cgpui::InvalidationState invalidation =
        test_context.invalidation_state();
    const cgpui::RuntimeDiagnosticsSnapshot diagnostics =
        test_context.diagnostics_snapshot();
    const std::span<const cgpui::PlatformDiagnosticEvent> diagnostics_stream =
        test_context.platform_diagnostics();

    (void)route;
    (void)action;
    (void)invalidation;
    (void)diagnostics;
    (void)diagnostics_stream;

    test_context.clear_invalidation();
    test_context.run_until_parked();
    test_context.advance_time(0);
    test_context.advance_time_until_parked(0);
    (void)test_context.cancel_timer(cgpui::TimerId{999});
    (void)test_context.complete_task(cgpui::TaskId{999});
    test_context.drain_task_completions();

    if (runtime_id.value == 0 || view_id.value == 0 || frame_index < 0 ||
        input.pointer_position.x < 0.0F) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<TestContextCapabilityView>&;
using Capability = cgpui::TestContextCapability;

static_assert(std::same_as<decltype(std::declval<ContextRef>()
                                        .test_context()),
                           Capability>);
static_assert(std::same_as<decltype(std::declval<Capability>().runtime_id()),
                           cgpui::WindowRuntimeId>);
static_assert(std::same_as<decltype(std::declval<Capability>().view_id()),
                           cgpui::ViewId>);
static_assert(std::same_as<decltype(std::declval<Capability>().frame_index()),
                           int>);
static_assert(std::same_as<decltype(std::declval<Capability>().input_state()),
                           cgpui::ViewInputState>);
static_assert(
    std::same_as<decltype(std::declval<Capability>().current_event_route()),
                 std::optional<cgpui::EventRoute>>);
static_assert(
    std::same_as<decltype(std::declval<Capability>().last_action_dispatch()),
                 std::optional<cgpui::ActionDispatchResult>>);
static_assert(
    std::same_as<decltype(std::declval<Capability>().invalidation_state()),
                 cgpui::InvalidationState>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .clear_invalidation()),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .diagnostics_snapshot()),
                           cgpui::RuntimeDiagnosticsSnapshot>);
static_assert(
    std::same_as<decltype(std::declval<Capability>().platform_diagnostics()),
                 std::span<const cgpui::PlatformDiagnosticEvent>>);
static_assert(std::same_as<decltype(std::declval<Capability>().advance_time(
                               std::uint64_t{})),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .run_until_parked()),
                           void>);
static_assert(
    std::same_as<decltype(std::declval<Capability>()
                              .advance_time_until_parked(std::uint64_t{})),
                 void>);
static_assert(std::same_as<decltype(std::declval<Capability>().cancel_timer(
                               std::declval<cgpui::TimerId>())),
                           bool>);
static_assert(std::same_as<decltype(std::declval<Capability>().complete_task(
                               std::declval<cgpui::TaskId>())),
                           bool>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .drain_task_completions()),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .dispatch_keystroke(
                                            std::declval<cgpui::KeyboardKey>())),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .simulate_keystrokes(
                                            std::declval<std::string_view>())),
                           bool>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .dispatch_pointer_move(
                                            std::declval<cgpui::Point>())),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .dispatch_pointer_button(
                                            std::declval<cgpui::MouseButton>(),
                                            bool{},
                                            std::declval<cgpui::Point>())),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .dispatch_pointer_scroll(
                                            std::declval<cgpui::Point>(),
                                            std::declval<cgpui::Point>())),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .dispatch_window_activation(bool{})),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .dispatch_window_focus(bool{})),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>().focus(
                               std::declval<cgpui::ElementId>())),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>().release_focus(
                               std::declval<cgpui::ElementId>())),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>().set_clipboard(
                               std::declval<cgpui::Clipboard*>())),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .write_to_clipboard(
                                            std::declval<std::string_view>())),
                           bool>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .read_from_clipboard()),
                           std::optional<std::string>>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .paste_clipboard_text()),
                           bool>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .copy_selection_to_clipboard()),
                           bool>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .cut_selection_to_clipboard()),
                           bool>);
static_assert(std::is_copy_constructible_v<Capability>);
static_assert(cgpui::Render<TestContextCapabilityView>);

} // namespace

int main() {
  return 0;
}

#include "cgpui/prelude.hpp"

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

class AsyncContextCapabilityView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<AsyncContextCapabilityView>& context) override {
    const cgpui::AsyncContextCapability async_context =
        context.async_context();

    async_context.defer([](const cgpui::WindowRuntimeContext& deferred) {
      deferred.request_render();
    });
    const cgpui::TimerId one_shot = async_context.schedule_timer(
        1,
        [](const cgpui::WindowRuntimeContext& timer_context) {
          timer_context.request_layout();
        });
    const cgpui::TimerId repeating = async_context.schedule_repeating_timer(
        2,
        [](const cgpui::WindowRuntimeContext& timer_context) {
          timer_context.request_paint();
        });

    const cgpui::AnimationHandle animation = async_context.start_animation(
        cgpui::AnimationOptions{.duration_ms = 8},
        [](const cgpui::WindowRuntimeContext&,
           const cgpui::AnimationSnapshot&) {});
    (void)async_context.animation_snapshot(animation.id());
    (void)async_context.cancel_animation(animation.id());

    const cgpui::TaskHandle task = async_context.spawn_task(
        [](const cgpui::WindowRuntimeContext& task_context) {
          task_context.request_render();
        });
    const cgpui::TaskHandle background_task =
        async_context.spawn_background_task(
            [](cgpui::TaskCancellationToken token) {
              (void)token.cancellation_requested();
            },
            [](const cgpui::WindowRuntimeContext& task_context) {
              task_context.request_paint();
            });

    async_context.batch_updates(
        [](const cgpui::WindowRuntimeContext& batch_context) {
          batch_context.request_layout();
        });

    if (one_shot.value == 0 || repeating.value == 0 ||
        task.id().value == 0 || background_task.id().value == 0) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<AsyncContextCapabilityView>&;
using Capability = cgpui::AsyncContextCapability;

static_assert(std::same_as<decltype(std::declval<ContextRef>()
                                        .async_context()),
                           Capability>);
static_assert(std::same_as<decltype(std::declval<Capability>().defer(
                               std::declval<cgpui::DeferredCallback>())),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>().schedule_timer(
                               std::uint64_t{},
                               std::declval<cgpui::TimerCallback>())),
                           cgpui::TimerId>);
static_assert(
    std::same_as<decltype(std::declval<Capability>().schedule_repeating_timer(
                     std::uint64_t{},
                     std::declval<cgpui::TimerCallback>())),
                 cgpui::TimerId>);
static_assert(std::same_as<decltype(std::declval<Capability>().start_animation(
                               std::declval<cgpui::AnimationOptions>(),
                               std::declval<cgpui::AnimationCallback>())),
                           cgpui::AnimationHandle>);
static_assert(
    std::same_as<decltype(std::declval<Capability>().animation_snapshot(
                     std::declval<cgpui::AnimationId>())),
                 std::optional<cgpui::AnimationSnapshot>>);
static_assert(std::same_as<decltype(std::declval<Capability>().cancel_animation(
                               std::declval<cgpui::AnimationId>())),
                           bool>);
static_assert(std::same_as<decltype(std::declval<Capability>().spawn_task(
                               std::declval<cgpui::TaskCompletionCallback>())),
                           cgpui::TaskHandle>);
static_assert(
    std::same_as<decltype(std::declval<Capability>().spawn_background_task(
                     std::declval<cgpui::BackgroundTaskCallback>(),
                     std::declval<cgpui::TaskCompletionCallback>())),
                 cgpui::TaskHandle>);
static_assert(std::same_as<decltype(std::declval<Capability>().batch_updates(
                               std::declval<cgpui::UpdateBatchCallback>())),
                           void>);
static_assert(std::is_copy_constructible_v<Capability>);
static_assert(cgpui::Render<AsyncContextCapabilityView>);

} // namespace

int main() {
  return 0;
}

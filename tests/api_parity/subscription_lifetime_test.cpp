#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

struct SubscriptionState {
  explicit SubscriptionState(int initial_value) : value(initial_value) {}

  int value = 0;
};

class SubscriptionLifetimeView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<SubscriptionLifetimeView>& context) override {
    const cgpui::EntityHandle<SubscriptionState> entity =
        context.new_entity<SubscriptionState>(3);

    int observer_count = 0;
    cgpui::Subscription subscription = entity.observe_subscription(
        context,
        [&observer_count](
            const cgpui::Context<SubscriptionLifetimeView>&,
            cgpui::EntityHandle<SubscriptionState>) {
          observer_count += 1;
        });

    if (!subscription.connected()) {
      return nullptr;
    }
    if (!entity.update(context, [](SubscriptionState& state) {
          state.value = 4;
        }) ||
        observer_count != 1) {
      return nullptr;
    }
    if (!subscription.release() || subscription.connected() ||
        subscription.release()) {
      return nullptr;
    }
    if (entity.update(context, [](SubscriptionState& state) {
          state.value = 5;
        }) ||
        observer_count != 1) {
      return nullptr;
    }

    {
      cgpui::Subscription scoped = context.observe_entity_subscription(
          entity,
          [&observer_count](
              const cgpui::Context<SubscriptionLifetimeView>&,
              cgpui::EntityHandle<SubscriptionState>) {
            observer_count += 10;
          });
      if (!scoped.connected()) {
        return nullptr;
      }
    }
    if (entity.update(context, [](SubscriptionState& state) {
          state.value = 6;
        }) ||
        observer_count != 1) {
      return nullptr;
    }

    cgpui::Subscription moved_from = context.observe_entity_subscription(
        entity,
        [&observer_count](
            const cgpui::Context<SubscriptionLifetimeView>&,
            cgpui::EntityHandle<SubscriptionState>) {
          observer_count += 100;
        });
    cgpui::Subscription moved_to = std::move(moved_from);
    if (moved_from.connected() || moved_from.release() ||
        !moved_to.connected()) {
      return nullptr;
    }
    if (!moved_to.release() || moved_to.connected() || moved_to.release()) {
      return nullptr;
    }
    if (entity.update(context, [](SubscriptionState& state) {
          state.value = 7;
        }) ||
        observer_count != 1) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<SubscriptionLifetimeView>&;
using Handle = cgpui::EntityHandle<SubscriptionState>;
using Observer = void (*)(
    const cgpui::Context<SubscriptionLifetimeView>&,
    cgpui::EntityHandle<SubscriptionState>);

static_assert(std::same_as<decltype(std::declval<cgpui::Subscription>()
                                        .connected()),
                           bool>);
static_assert(std::same_as<decltype(std::declval<cgpui::Subscription>()
                                        .release()),
                           bool>);
static_assert(std::same_as<
              decltype(std::declval<Handle>().observe_subscription(
                  std::declval<ContextRef>(),
                  std::declval<Observer>())),
              cgpui::Subscription>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().observe_entity_subscription(
                  std::declval<Handle>(),
                  std::declval<Observer>())),
              cgpui::Subscription>);
static_assert(!std::is_copy_constructible_v<cgpui::Subscription>);
static_assert(!std::is_copy_assignable_v<cgpui::Subscription>);
static_assert(std::is_move_constructible_v<cgpui::Subscription>);
static_assert(std::is_move_assignable_v<cgpui::Subscription>);
static_assert(cgpui::Render<SubscriptionLifetimeView>);

} // namespace

int main() {
  cgpui::Subscription empty;
  return !empty.connected() && !empty.release() ? 0 : 1;
}

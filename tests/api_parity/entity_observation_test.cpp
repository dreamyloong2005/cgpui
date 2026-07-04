#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

struct ObservedState {
  explicit ObservedState(int initial_value) : value(initial_value) {}

  int value = 0;
};

class EntityObservationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<EntityObservationView>& context) override {
    const cgpui::EntityHandle<ObservedState> entity =
        context.new_entity<ObservedState>(5);

    int observed_value = 0;
    const bool observed = entity.observe(
        context,
        [&observed_value](
            const cgpui::Context<EntityObservationView>& observer_context,
            cgpui::EntityHandle<ObservedState> observed_entity) {
          const ObservedState* state = observed_entity.read(observer_context);
          observed_value = state == nullptr ? -1 : state->value;
        });
    if (!observed) {
      return nullptr;
    }

    cgpui::Subscription subscription = entity.observe_subscription(
        context,
        [](const cgpui::Context<EntityObservationView>& observer_context,
           cgpui::EntityHandle<ObservedState> observed_entity) {
          (void)observed_entity.read(observer_context);
        });
    if (!subscription.connected()) {
      return nullptr;
    }

    cgpui::Subscription context_subscription =
        context.observe_entity_subscription(
            entity,
            [](const cgpui::Context<EntityObservationView>& observer_context,
               cgpui::EntityHandle<ObservedState> observed_entity) {
              (void)observed_entity.read(observer_context);
            });
    if (!context_subscription.connected()) {
      return nullptr;
    }

    const bool context_observed = context.observe_entity(
        entity,
        [&observed_value](
            const cgpui::Context<EntityObservationView>& observer_context,
            cgpui::EntityHandle<ObservedState> observed_entity) {
          const ObservedState* state = observed_entity.read(observer_context);
          observed_value += state == nullptr ? -10 : state->value;
        });
    if (!context_observed) {
      return nullptr;
    }

    if (!entity.update(context, [](ObservedState& state) {
          state.value = 13;
        })) {
      return nullptr;
    }

    if (observed_value != 26) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<EntityObservationView>&;
using HandleRef = const cgpui::EntityHandle<ObservedState>&;
using Observer = void (*)(
    const cgpui::Context<EntityObservationView>&,
    cgpui::EntityHandle<ObservedState>);

static_assert(std::same_as<
              decltype(std::declval<HandleRef>().observe(
                  std::declval<ContextRef>(),
                  std::declval<Observer>())),
              bool>);
static_assert(std::same_as<
              decltype(std::declval<HandleRef>().observe_subscription(
                  std::declval<ContextRef>(),
                  std::declval<Observer>())),
              cgpui::Subscription>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().observe_entity(
                  std::declval<cgpui::EntityHandle<ObservedState>>(),
                  std::declval<Observer>())),
              bool>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().observe_entity_subscription(
                  std::declval<cgpui::EntityHandle<ObservedState>>(),
                  std::declval<Observer>())),
              cgpui::Subscription>);
static_assert(cgpui::Render<EntityObservationView>);
static_assert(std::is_move_constructible_v<cgpui::Subscription>);

} // namespace

int main() {
  const cgpui::EntityHandle<ObservedState> empty;
  return empty.empty() && empty.id().value == 0 ? 0 : 1;
}

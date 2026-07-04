#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

struct ObserverState {
  explicit ObserverState(int initial_value) : observed_total(initial_value) {}

  int observed_total = 0;
};

struct ObservedState {
  explicit ObservedState(int initial_value) : value(initial_value) {}

  int value = 0;
};

class EntityToEntityObservationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<EntityToEntityObservationView>& context) override {
    const cgpui::EntityHandle<ObserverState> observer =
        context.new_entity<ObserverState>(0);
    const cgpui::EntityHandle<ObservedState> observed =
        context.new_entity<ObservedState>(4);

    const bool observing = observer.observe_entity(
        context,
        observed,
        [](ObserverState& owner,
           cgpui::EntityHandle<ObservedState> observed_entity,
           const cgpui::Context<ObserverState>& observer_context) {
          const ObservedState* state = observed_entity.read(observer_context);
          owner.observed_total += state == nullptr ? -100 : state->value;
        });
    if (!observing) {
      return nullptr;
    }

    cgpui::Subscription subscription =
        context.observe_entity_subscription(
            observer,
            observed,
            [](ObserverState& owner,
               cgpui::EntityHandle<ObservedState> observed_entity,
               const cgpui::Context<ObserverState>& observer_context) {
              const ObservedState* state =
                  observed_entity.read(observer_context);
              owner.observed_total += state == nullptr ? -1000 : state->value;
            });
    if (!subscription.connected()) {
      return nullptr;
    }

    if (!observed.update(context, [](ObservedState& state) {
          state.value = 9;
        })) {
      return nullptr;
    }

    const ObserverState* owner = observer.read(context);
    if (owner == nullptr || owner->observed_total != 18) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<EntityToEntityObservationView>&;
using ObserverHandle = cgpui::EntityHandle<ObserverState>;
using ObservedHandle = cgpui::EntityHandle<ObservedState>;
using EntityObserver = void (*)(
    ObserverState&,
    cgpui::EntityHandle<ObservedState>,
    const cgpui::Context<ObserverState>&);

static_assert(std::same_as<
              decltype(std::declval<ObserverHandle>().observe_entity(
                  std::declval<ContextRef>(),
                  std::declval<ObservedHandle>(),
                  std::declval<EntityObserver>())),
              bool>);
static_assert(std::same_as<
              decltype(std::declval<ObserverHandle>()
                           .observe_entity_subscription(
                               std::declval<ContextRef>(),
                               std::declval<ObservedHandle>(),
                               std::declval<EntityObserver>())),
              cgpui::Subscription>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().observe_entity(
                  std::declval<ObserverHandle>(),
                  std::declval<ObservedHandle>(),
                  std::declval<EntityObserver>())),
              bool>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>()
                           .observe_entity_subscription(
                               std::declval<ObserverHandle>(),
                               std::declval<ObservedHandle>(),
                               std::declval<EntityObserver>())),
              cgpui::Subscription>);
static_assert(cgpui::Render<EntityToEntityObservationView>);

} // namespace

int main() {
  const cgpui::EntityHandle<ObserverState> empty;
  return empty.empty() && empty.id().value == 0 ? 0 : 1;
}

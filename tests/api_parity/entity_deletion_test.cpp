#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

struct DeletionState {
  explicit DeletionState(int initial_value) : value(initial_value) {}

  int value = 0;
};

class EntityDeletionView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<EntityDeletionView>& context) override {
    const cgpui::EntityHandle<DeletionState> entity =
        context.new_entity<DeletionState>(17);
    const cgpui::WeakEntity<DeletionState> weak = entity.downgrade();

    if (!entity.remove(context)) {
      return nullptr;
    }
    if (entity.read(context) != nullptr || weak.upgrade(context).has_value()) {
      return nullptr;
    }
    if (entity.update(context, [](DeletionState& state) {
          state.value = 23;
        }) ||
        entity.invalidate(context)) {
      return nullptr;
    }

    const cgpui::EntityHandle<DeletionState> context_entity =
        context.new_entity<DeletionState>(31);
    if (!context.remove_entity(context_entity)) {
      return nullptr;
    }
    if (context.remove_entity(context_entity)) {
      return nullptr;
    }

    const cgpui::EntityHandle<DeletionState> missing(
        cgpui::EntityId<DeletionState>{context_entity.id().value + 100});
    const cgpui::EntityHandle<DeletionState> empty;
    if (missing.remove(context) || context.remove_entity(empty)) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<EntityDeletionView>&;
using Handle = cgpui::EntityHandle<DeletionState>;

static_assert(std::same_as<
              decltype(std::declval<Handle>().remove(
                  std::declval<ContextRef>())),
              bool>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().remove_entity(
                  std::declval<Handle>())),
              bool>);
static_assert(cgpui::Render<EntityDeletionView>);
static_assert(std::is_move_constructible_v<Handle>);

} // namespace

int main() {
  const cgpui::EntityHandle<DeletionState> empty;
  return empty.empty() && empty.id().value == 0 ? 0 : 1;
}

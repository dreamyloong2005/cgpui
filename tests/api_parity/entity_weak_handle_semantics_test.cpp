#include "cgpui/prelude.hpp"

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

struct WeakLifecycleState {
  explicit WeakLifecycleState(int initial_value) : value(initial_value) {}

  int value = 0;
};

class EntityWeakHandleSemanticsView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<EntityWeakHandleSemanticsView>& context) override {
    const cgpui::EntityHandle<WeakLifecycleState> entity =
        context.new_entity<WeakLifecycleState>(17);
    const cgpui::WeakEntity<WeakLifecycleState> weak = entity.downgrade();

    const std::optional<cgpui::EntityHandle<WeakLifecycleState>> upgraded =
        weak.upgrade(context);
    if (!upgraded.has_value() || upgraded->id() != entity.id()) {
      return nullptr;
    }

    const WeakLifecycleState* initial = weak.read(context);
    if (initial == nullptr || initial->value != 17) {
      return nullptr;
    }

    if (!upgraded->update(context, [](WeakLifecycleState& state) {
          state.value = 23;
        })) {
      return nullptr;
    }

    const WeakLifecycleState* updated = weak.read(context);
    if (updated == nullptr || updated->value != 23) {
      return nullptr;
    }

    (void)context.remove_entity(entity.id());
    if (weak.upgrade(context).has_value() || weak.read(context) != nullptr) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<EntityWeakHandleSemanticsView>&;
using WeakRef = const cgpui::WeakEntity<WeakLifecycleState>&;

static_assert(std::same_as<
              decltype(std::declval<WeakRef>().upgrade(
                  std::declval<ContextRef>())),
              std::optional<cgpui::EntityHandle<WeakLifecycleState>>>);
static_assert(std::same_as<
              decltype(std::declval<WeakRef>().read(
                  std::declval<ContextRef>())),
              const WeakLifecycleState*>);
static_assert(cgpui::Render<EntityWeakHandleSemanticsView>);
static_assert(std::is_copy_constructible_v<
              cgpui::WeakEntity<WeakLifecycleState>>);

} // namespace

int main() {
  const cgpui::WeakEntity<WeakLifecycleState> empty;
  return empty.empty() && empty.id().value == 0 ? 0 : 1;
}

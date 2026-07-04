#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

struct LifecycleState {
  explicit LifecycleState(int initial_value) : value(initial_value) {}

  int value = 0;
};

class EntityLifecycleCreationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<EntityLifecycleCreationView>& context) override {
    const cgpui::EntityHandle<LifecycleState> entity =
        context.new_entity<LifecycleState>(7);

    const LifecycleState* initial = entity.read(context);
    if (initial == nullptr || initial->value != 7) {
      return nullptr;
    }

    if (!entity.update(context, [](LifecycleState& state) {
          state.value = 11;
        })) {
      return nullptr;
    }

    const LifecycleState* updated = entity.read(context);
    if (updated == nullptr || updated->value != 11) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<EntityLifecycleCreationView>&;

static_assert(std::same_as<
              decltype(std::declval<ContextRef>().new_entity<LifecycleState>(
                  7)),
              cgpui::EntityHandle<LifecycleState>>);
static_assert(cgpui::Render<EntityLifecycleCreationView>);
static_assert(std::is_move_constructible_v<
              cgpui::EntityHandle<LifecycleState>>);

} // namespace

int main() {
  const cgpui::EntityHandle<LifecycleState> empty;
  return empty.empty() && empty.id().value == 0 ? 0 : 1;
}

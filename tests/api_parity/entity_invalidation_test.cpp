#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

struct InvalidationStateModel {
  explicit InvalidationStateModel(int initial_value) : value(initial_value) {}

  int value = 0;
};

class EntityInvalidationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<EntityInvalidationView>& context) override {
    const cgpui::EntityHandle<InvalidationStateModel> entity =
        context.new_entity<InvalidationStateModel>(17);

    if (!entity.invalidate(context)) {
      return nullptr;
    }

    if (!context.invalidate_entity(entity)) {
      return nullptr;
    }

    const cgpui::EntityHandle<InvalidationStateModel> missing(
        cgpui::EntityId<InvalidationStateModel>{entity.id().value + 100});
    if (missing.invalidate(context) || context.invalidate_entity(missing)) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<EntityInvalidationView>&;
using Handle = cgpui::EntityHandle<InvalidationStateModel>;

static_assert(std::same_as<
              decltype(std::declval<Handle>().invalidate(
                  std::declval<ContextRef>())),
              bool>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().invalidate_entity(
                  std::declval<Handle>())),
              bool>);
static_assert(cgpui::Render<EntityInvalidationView>);
static_assert(std::is_move_constructible_v<Handle>);

} // namespace

int main() {
  const cgpui::EntityHandle<InvalidationStateModel> empty;
  return empty.empty() && empty.id().value == 0 ? 0 : 1;
}

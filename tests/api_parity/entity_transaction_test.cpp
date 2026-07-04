#include "cgpui/prelude.hpp"

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

struct TransactionState {
  explicit TransactionState(int initial_value) : value(initial_value) {}

  int value = 0;
};

class EntityUpdateTransactionView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<EntityUpdateTransactionView>& context) override {
    const cgpui::EntityHandle<TransactionState> entity =
        context.new_entity<TransactionState>(3);

    const std::optional<int> returned = entity.update(
        context,
        [](TransactionState& state,
           const cgpui::Context<TransactionState>& transaction_context) {
          (void)transaction_context.app();
          state.value += 7;
          return state.value * 2;
        });
    if (!returned.has_value() || *returned != 20) {
      return nullptr;
    }

    if (!context.update_entity(
            entity,
            [](TransactionState& state,
               const cgpui::Context<TransactionState>& transaction_context) {
              (void)transaction_context.window();
              state.value += 1;
            })) {
      return nullptr;
    }

    const TransactionState* updated = entity.read(context);
    if (updated == nullptr || updated->value != 11) {
      return nullptr;
    }

    const cgpui::EntityHandle<TransactionState> missing(
        cgpui::EntityId<TransactionState>{9999});
    const std::optional<int> missing_result = context.update_entity(
        missing,
        [](TransactionState& state,
           const cgpui::Context<TransactionState>& transaction_context) {
          (void)transaction_context;
          return state.value;
        });
    if (missing_result.has_value()) {
      return nullptr;
    }

    if (missing.update(
            context,
            [](TransactionState& state,
               const cgpui::Context<TransactionState>& transaction_context) {
              (void)transaction_context;
              state.value += 1;
            })) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<EntityUpdateTransactionView>&;
using EntityContextRef = const cgpui::Context<TransactionState>&;
using Handle = cgpui::EntityHandle<TransactionState>;
using ValueTransaction = int (*)(TransactionState&, EntityContextRef);
using VoidTransaction = void (*)(TransactionState&, EntityContextRef);

static_assert(std::same_as<
              decltype(std::declval<Handle>().update(
                  std::declval<ContextRef>(),
                  std::declval<ValueTransaction>())),
              std::optional<int>>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().update_entity(
                  std::declval<Handle>(),
                  std::declval<ValueTransaction>())),
              std::optional<int>>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().update_entity(
                  std::declval<Handle>(),
                  std::declval<VoidTransaction>())),
              bool>);
static_assert(cgpui::Render<EntityUpdateTransactionView>);

} // namespace

int main() {
  const cgpui::EntityHandle<TransactionState> empty;
  return empty.empty() && empty.id().value == 0 ? 0 : 1;
}

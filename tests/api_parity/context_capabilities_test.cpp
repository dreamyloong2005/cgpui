#include "cgpui/cgpui.hpp"

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

struct ContextState {
  int value = 0;
};

class ContextCapabilitiesView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<ContextCapabilitiesView>& context) override {
    const cgpui::App app = context.app();
    const cgpui::Window window = context.window();
    const cgpui::Window current_window = context.current_window();
    const cgpui::AsyncContextCapability async_context =
        context.async_context();
    const cgpui::TestContextCapability test_context =
        context.test_context();
    (void)app;
    (void)window;
    (void)current_window;
    (void)async_context;
    (void)test_context;

    const cgpui::Model<ContextState> model =
        context.new_model<ContextState>(7);
    const cgpui::EntityHandle<ContextState> entity = context.entity(model);
    const cgpui::WeakEntity<ContextState> weak_entity =
        context.weak_entity(model);

    const ContextState* initial = entity.read(context);
    if (initial == nullptr || initial->value != 7) {
      return nullptr;
    }
    if (!entity.update(context, [](ContextState& state) {
          state.value = 11;
        })) {
      return nullptr;
    }

    const std::optional<cgpui::Model<ContextState>> upgraded =
        context.upgrade_entity(weak_entity);
    if (!upgraded.has_value() || upgraded->value != model.value) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<ContextCapabilitiesView>&;

static_assert(std::same_as<decltype(std::declval<ContextRef>().app()),
                           cgpui::App>);
static_assert(std::same_as<decltype(std::declval<ContextRef>().window()),
                           cgpui::Window>);
static_assert(
    std::same_as<decltype(std::declval<ContextRef>().current_window()),
                 cgpui::Window>);
static_assert(std::same_as<decltype(std::declval<ContextRef>().async_context()),
                           cgpui::AsyncContextCapability>);
static_assert(std::same_as<decltype(std::declval<ContextRef>().test_context()),
                           cgpui::TestContextCapability>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().entity(
                  cgpui::Model<ContextState>{1})),
              cgpui::EntityHandle<ContextState>>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().weak_entity(
                  cgpui::Model<ContextState>{1})),
              cgpui::WeakEntity<ContextState>>);
static_assert(cgpui::Render<ContextCapabilitiesView>);

} // namespace

int main() {
  return 0;
}

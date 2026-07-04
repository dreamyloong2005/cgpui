#include "cgpui/prelude.hpp"

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

class ViewContextCapabilityView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<ViewContextCapabilityView>& context) override {
    const cgpui::ViewContextCapability<ViewContextCapabilityView>
        view_context =
            context.view_context<ViewContextCapabilityView>();
    const cgpui::ViewHandle<ViewContextCapabilityView> view =
        view_context.view();
    const cgpui::WeakViewHandle<ViewContextCapabilityView> weak_view =
        view_context.weak_view();
    const std::optional<cgpui::ViewHandle<ViewContextCapabilityView>>
        upgraded = view_context.upgrade(weak_view);
    const ViewContextCapabilityView* current = view_context.current();

    if (view_context.view_id() != context.view_id || view.empty() ||
        view.id() != context.view_id || weak_view.empty() ||
        weak_view.id() != context.view_id || !upgraded.has_value() ||
        upgraded->id() != view.id() || current == nullptr ||
        view_context.read(view) != current) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<ViewContextCapabilityView>&;
using ViewCapability =
    cgpui::ViewContextCapability<ViewContextCapabilityView>;
using ViewHandle = cgpui::ViewHandle<ViewContextCapabilityView>;
using WeakViewHandle = cgpui::WeakViewHandle<ViewContextCapabilityView>;

static_assert(std::same_as<decltype(std::declval<ContextRef>()
                                        .view_context<ViewContextCapabilityView>()),
                           ViewCapability>);
static_assert(std::same_as<decltype(std::declval<ViewCapability>().view_id()),
                           cgpui::ViewId>);
static_assert(std::same_as<decltype(std::declval<ViewCapability>().view()),
                           ViewHandle>);
static_assert(std::same_as<decltype(std::declval<ViewCapability>().weak_view()),
                           WeakViewHandle>);
static_assert(std::same_as<decltype(std::declval<ViewCapability>().upgrade(
                               std::declval<WeakViewHandle>())),
                           std::optional<ViewHandle>>);
static_assert(std::same_as<decltype(std::declval<ViewCapability>().read(
                               std::declval<ViewHandle>())),
                           const ViewContextCapabilityView*>);
static_assert(std::same_as<decltype(std::declval<ViewCapability>().current()),
                           const ViewContextCapabilityView*>);
static_assert(std::is_copy_constructible_v<ViewCapability>);
static_assert(cgpui::Render<ViewContextCapabilityView>);

} // namespace

int main() {
  return 0;
}

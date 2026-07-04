#include "cgpui/cgpui.hpp"

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

class ViewHandleSpellingView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<ViewHandleSpellingView>& context) override {
    const cgpui::ViewHandle<ViewHandleSpellingView> view =
        context.view<ViewHandleSpellingView>();
    const cgpui::WeakViewHandle<ViewHandleSpellingView> weak_view =
        view.downgrade();
    const cgpui::WeakViewHandle<ViewHandleSpellingView> context_weak_view =
        context.weak_view<ViewHandleSpellingView>();
    const std::optional<cgpui::ViewHandle<ViewHandleSpellingView>> upgraded =
        context.upgrade_view(weak_view);
    const ViewHandleSpellingView* current_view = view.read(context);

    if (view.empty() || view.id() != context.view_id ||
        weak_view.empty() || weak_view.id() != context.view_id ||
        context_weak_view.id() != context.view_id ||
        !upgraded.has_value() || upgraded->id() != view.id() ||
        current_view == nullptr ||
        context.read_view(view) != current_view ||
        weak_view.untyped().id() != context.view_id) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ViewContextRef = cgpui::Context<ViewHandleSpellingView>&;
using ViewHandle = cgpui::ViewHandle<ViewHandleSpellingView>;
using WeakViewHandle = cgpui::WeakViewHandle<ViewHandleSpellingView>;

static_assert(std::same_as<decltype(std::declval<ViewContextRef>()
                                        .view<ViewHandleSpellingView>()),
                           ViewHandle>);
static_assert(std::same_as<decltype(std::declval<ViewContextRef>()
                                        .weak_view<ViewHandleSpellingView>()),
                           WeakViewHandle>);
static_assert(std::same_as<decltype(std::declval<ViewContextRef>().upgrade_view(
                               std::declval<WeakViewHandle>())),
                           std::optional<ViewHandle>>);
static_assert(std::same_as<decltype(std::declval<ViewHandle>().downgrade()),
                           WeakViewHandle>);
static_assert(std::same_as<decltype(std::declval<WeakViewHandle>().untyped()),
                           cgpui::WeakView>);
static_assert(std::same_as<decltype(std::declval<ViewHandle>().read(
                               std::declval<ViewContextRef>())),
                           const ViewHandleSpellingView*>);
static_assert(cgpui::Render<ViewHandleSpellingView>);

} // namespace

int main() {
  return 0;
}

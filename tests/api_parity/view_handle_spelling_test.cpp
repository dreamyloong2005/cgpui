#include "cgpui/cgpui.hpp"

#include <concepts>
#include <cstdint>
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
        view.context_token() == 0 ||
        weak_view.context_token() != view.context_token() ||
        context_weak_view.context_token() != view.context_token() ||
        upgraded->context_token() != view.context_token() ||
        current_view == nullptr ||
        context.read_view(view) != current_view ||
        weak_view.untyped().id() != context.view_id ||
        weak_view.untyped().context_token() != view.context_token() ||
        !view.matches_context(view.context_token()) ||
        !weak_view.matches_context(view.context_token()) ||
        !weak_view.untyped().matches_context(view.context_token())) {
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
static_assert(std::same_as<decltype(std::declval<ViewHandle>().context_token()),
                           std::uintptr_t>);
static_assert(std::same_as<decltype(std::declval<WeakViewHandle>()
                                        .context_token()),
                           std::uintptr_t>);
static_assert(std::same_as<decltype(std::declval<cgpui::WeakView>()
                                        .context_token()),
                           std::uintptr_t>);
static_assert(std::same_as<decltype(std::declval<ViewHandle>().matches_context(
                               std::declval<std::uintptr_t>())),
                           bool>);
static_assert(std::same_as<decltype(std::declval<WeakViewHandle>()
                                        .matches_context(
                                            std::declval<std::uintptr_t>())),
                           bool>);
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

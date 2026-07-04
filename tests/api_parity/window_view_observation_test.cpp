#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

class WindowViewObservationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<WindowViewObservationView>& context) override {
    const cgpui::WindowContextCapability window_context =
        context.window_context();
    const cgpui::ViewContextCapability<WindowViewObservationView>
        view_context = context.view_context<WindowViewObservationView>();
    const cgpui::ViewHandle<WindowViewObservationView> view =
        view_context.view();

    const bool window_observed =
        context.observe_window(
            [](const cgpui::Context<WindowViewObservationView>&,
               cgpui::WindowContextCapability window) {
              (void)window.runtime_id();
            });
    cgpui::Subscription window_subscription =
        window_context.observe_subscription(
            [](const cgpui::Context<WindowViewObservationView>&,
               cgpui::WindowContextCapability window) {
              (void)window.viewport_size();
            });

    const bool view_observed =
        view.observe(
            context,
            [](const cgpui::Context<WindowViewObservationView>&,
               cgpui::ViewHandle<WindowViewObservationView> observed) {
              (void)observed.id();
            });
    cgpui::Subscription view_subscription =
        view_context.observe_subscription(
            view,
            [](const cgpui::Context<WindowViewObservationView>&,
               cgpui::ViewHandle<WindowViewObservationView> observed) {
              (void)observed.empty();
            });

    if (!window_observed || !window_subscription.connected() ||
        !view_observed || !view_subscription.connected()) {
      return nullptr;
    }

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<WindowViewObservationView>&;
using ViewHandle = cgpui::ViewHandle<WindowViewObservationView>;
using ViewCapability =
    cgpui::ViewContextCapability<WindowViewObservationView>;
using WindowObserver = void (*)(
    const cgpui::Context<WindowViewObservationView>&,
    cgpui::WindowContextCapability);
using ViewObserver = void (*)(
    const cgpui::Context<WindowViewObservationView>&,
    cgpui::ViewHandle<WindowViewObservationView>);

static_assert(std::same_as<decltype(std::declval<ContextRef>().observe_window(
                               std::declval<WindowObserver>())),
                           bool>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().observe_window_subscription(
                  std::declval<WindowObserver>())),
              cgpui::Subscription>);
static_assert(std::same_as<
              decltype(std::declval<cgpui::WindowContextCapability>().observe(
                  std::declval<WindowObserver>())),
              bool>);
static_assert(std::same_as<
              decltype(std::declval<cgpui::WindowContextCapability>()
                           .observe_subscription(
                               std::declval<WindowObserver>())),
              cgpui::Subscription>);
static_assert(std::same_as<decltype(std::declval<ContextRef>().observe_view(
                               std::declval<ViewHandle>(),
                               std::declval<ViewObserver>())),
                           bool>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().observe_view_subscription(
                  std::declval<ViewHandle>(),
                  std::declval<ViewObserver>())),
              cgpui::Subscription>);
static_assert(std::same_as<decltype(std::declval<ViewHandle>().observe(
                               std::declval<ContextRef>(),
                               std::declval<ViewObserver>())),
                           bool>);
static_assert(std::same_as<
              decltype(std::declval<ViewHandle>().observe_subscription(
                  std::declval<ContextRef>(),
                  std::declval<ViewObserver>())),
              cgpui::Subscription>);
static_assert(std::same_as<decltype(std::declval<ViewCapability>().observe(
                               std::declval<ViewHandle>(),
                               std::declval<ViewObserver>())),
                           bool>);
static_assert(std::same_as<
              decltype(std::declval<ViewCapability>().observe_subscription(
                  std::declval<ViewHandle>(),
                  std::declval<ViewObserver>())),
              cgpui::Subscription>);
static_assert(cgpui::Render<WindowViewObservationView>);

} // namespace

int main() {
  return 0;
}

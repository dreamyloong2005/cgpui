#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

struct ElementCapabilityState {
  int value = 0;
};

class ElementContextCapabilityView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<ElementContextCapabilityView>& context) override {
    const cgpui::ElementId element_id{7};
    const cgpui::ElementContextCapability element_context =
        context.element_context(element_id);

    if (element_context.element_id() != element_id) {
      return nullptr;
    }

    element_context.request_keyboard_focus();
    element_context.focus();
    (void)element_context.focus_handle();
    element_context.capture_pointer();
    element_context.set_cursor(cgpui::CursorShape::pointing_hand);
    element_context.release_pointer();
    element_context.release_keyboard_focus();
    element_context.blur();

    (void)element_context.state<ElementCapabilityState>();
    (void)element_context.emplace_state<ElementCapabilityState>(
        ElementCapabilityState{.value = 1});
    (void)element_context.state_or_init<ElementCapabilityState>(
        ElementCapabilityState{.value = 2});

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<ElementContextCapabilityView>&;
using Capability = cgpui::ElementContextCapability;

static_assert(std::same_as<decltype(std::declval<ContextRef>()
                                        .element_context(
                                            std::declval<cgpui::ElementId>())),
                           Capability>);
static_assert(std::same_as<decltype(std::declval<Capability>().element_id()),
                           cgpui::ElementId>);
static_assert(std::same_as<decltype(std::declval<Capability>().focus()), void>);
static_assert(std::same_as<decltype(std::declval<Capability>().blur()), void>);
static_assert(
    std::same_as<decltype(std::declval<Capability>()
                              .request_keyboard_focus()),
                 void>);
static_assert(
    std::same_as<decltype(std::declval<Capability>()
                              .release_keyboard_focus()),
                 void>);
static_assert(std::same_as<decltype(std::declval<Capability>().focus_handle()),
                           cgpui::FocusHandle>);
static_assert(
    std::same_as<decltype(std::declval<Capability>().capture_pointer()), void>);
static_assert(
    std::same_as<decltype(std::declval<Capability>().release_pointer()), void>);
static_assert(std::same_as<decltype(std::declval<Capability>().set_cursor(
                               cgpui::CursorShape::pointing_hand)),
                           void>);
static_assert(std::same_as<decltype(std::declval<Capability>()
                                        .state<ElementCapabilityState>()),
                           ElementCapabilityState*>);
static_assert(
    std::same_as<decltype(std::declval<Capability>()
                              .emplace_state<ElementCapabilityState>(
                                  std::declval<ElementCapabilityState>())),
                 ElementCapabilityState*>);
static_assert(
    std::same_as<decltype(std::declval<Capability>()
                              .state_or_init<ElementCapabilityState>(
                                  std::declval<ElementCapabilityState>())),
                 ElementCapabilityState*>);
static_assert(std::is_copy_constructible_v<Capability>);
static_assert(cgpui::Render<ElementContextCapabilityView>);

} // namespace

int main() {
  return 0;
}

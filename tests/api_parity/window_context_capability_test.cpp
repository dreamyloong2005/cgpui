#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

class WindowContextCapabilityView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<WindowContextCapabilityView>& context) override {
    const cgpui::WindowContextCapability window_context =
        context.window_context();
    const cgpui::Window window = window_context.window();
    const cgpui::Window current_window = window_context.current_window();
    const cgpui::WindowDescriptor descriptor = window_context.descriptor();
    const cgpui::Size viewport_size = window_context.viewport_size();
    const cgpui::DpiScale scale = window_context.scale();
    const cgpui::ViewInputState input_state = window_context.input_state();
    const bool focused = window_context.focused();
    const bool active = window_context.active();

    if (window_context.runtime_id() != window.runtime_id() ||
        current_window.runtime_id() != window.runtime_id() ||
        window_context.root_view_id() != window.root_view_id() ||
        descriptor.title != window.descriptor().title ||
        viewport_size.width != window.viewport_size().width ||
        viewport_size.height != window.viewport_size().height ||
        scale.value != window.scale().value ||
        input_state.focused != window.input_state().focused ||
        focused != window.focused() || active != window.active()) {
      return nullptr;
    }

    window_context.request_render();
    window_context.request_layout();
    window_context.request_paint();

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<WindowContextCapabilityView>&;

static_assert(std::same_as<decltype(std::declval<ContextRef>()
                                        .window_context()),
                           cgpui::WindowContextCapability>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .window()),
                 cgpui::Window>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .current_window()),
                 cgpui::Window>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .runtime_id()),
                 cgpui::WindowRuntimeId>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .descriptor()),
                 cgpui::WindowDescriptor>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .root_view_id()),
                 cgpui::ViewId>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .active()),
                 bool>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .viewport_size()),
                 cgpui::Size>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .scale()),
                 cgpui::DpiScale>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .input_state()),
                 cgpui::ViewInputState>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                              .focused()),
                 bool>);
static_assert(std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                                        .request_render()),
                           void>);
static_assert(std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                                        .request_layout()),
                           void>);
static_assert(std::same_as<decltype(std::declval<cgpui::WindowContextCapability>()
                                        .request_paint()),
                           void>);
static_assert(std::is_copy_constructible_v<cgpui::WindowContextCapability>);
static_assert(cgpui::Render<WindowContextCapabilityView>);

} // namespace

int main() {
  return 0;
}

#include "cgpui/ui/element_core.hpp"

namespace cgpui {

void Element::paint(PaintList& paint_list) const {
  (void)paint_list;
}

void Element::inherit_text_style(const Style& style) {
  (void)style;
}

void Element::focus(const ElementFocusContext& context) {
  (void)context;
}

void Element::on_mount(const ElementLifecycleContext& context) {
  (void)context;
}

void Element::on_update(const ElementLifecycleContext& context) {
  (void)context;
}

void Element::on_unmount(const ElementLifecycleContext& context) {
  (void)context;
}

EventResult Element::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  (void)event;
  (void)context;
  return EventResult::unhandled();
}

} // namespace cgpui

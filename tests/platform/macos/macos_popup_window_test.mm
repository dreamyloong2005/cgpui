#include "macos_window_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <cmath>

namespace {

using cgpui::Rect;
using cgpui::WindowPopupSide;

bool near(float a, float b) { return std::fabs(a - b) < 0.5F; }

bool same(Rect rect, float x, float y, float width, float height) {
  return near(rect.origin.x, x) && near(rect.origin.y, y) &&
         near(rect.size.width, width) && near(rect.size.height, height);
}

}  // namespace

int main() {
  @autoreleasepool {
    auto application = cgpui::create_platform_application();
    if (!application || !(*application)->popup_windows_supported()) return 10;
    auto parent = (*application)->create_window(
        cgpui::WindowDescriptor{.title = "CGPUI macOS Popup Test", .size = {400.0F, 300.0F}},
        [](const cgpui::PlatformEvent&) {});
    if (!parent) return 11;
    NSWindow* parent_native = dynamic_cast<cgpui::MacOSWindow*>(parent->get())->native_window();
    [parent_native setFrameOrigin:NSMakePoint(200, 200)];

    const cgpui::WindowDescriptor descriptor{
        .size = {120.0F, 80.0F},
        .kind = cgpui::WindowKind::popup,
        .popup = cgpui::WindowPopupPlacement{
            .anchor = {{10, 10}, {40, 20}}, .side = WindowPopupSide::below, .gap = 4}};
    if ((*application)->create_window(descriptor, [](const cgpui::PlatformEvent&) {})) {
      return 12;  // a popup needs a parent
    }
    auto popup = (*application)->create_child_window(
        descriptor, **parent, [](const cgpui::PlatformEvent&) {});
    if (!popup) return 13;
    auto* macos_popup = dynamic_cast<cgpui::MacOSWindow*>(popup->get());
    NSWindow* panel = macos_popup->native_window();
    if (![panel isKindOfClass:[NSPanel class]] || !macos_popup->popup() ||
        ([panel styleMask] & NSWindowStyleMaskNonactivatingPanel) == 0 ||
        [panel level] != NSPopUpMenuWindowLevel || [panel isKeyWindow] ||
        ![panel isVisible] || [panel isOpaque] || ![panel hidesOnDeactivate] ||
        ![[parent_native childWindows] containsObject:panel]) {
      return 14;
    }
    if ((*popup)->state().framebuffer_size.width < 120.0F) return 15;

    // Placed below the anchor, in the parent's client coordinates.
    const auto placed = (*popup)->place_popup(
        {.anchor = {{10, 10}, {40, 20}}, .side = WindowPopupSide::below, .gap = 4});
    if (!placed.supported || !placed.placed || placed.flipped ||
        !same(placed.bounds, 10, 34, 120, 80)) {
      return 16;
    }
    // Child windows follow their parent.
    const NSPoint before = [panel frame].origin;
    [parent_native setFrameOrigin:NSMakePoint(260, 230)];
    const NSPoint after = [panel frame].origin;
    if (!near(static_cast<float>(after.x - before.x), 60) ||
        !near(static_cast<float>(after.y - before.y), 30)) {
      return 17;
    }
    // Normal windows are not popups.
    return !(*parent)->place_popup({}).supported ? 0 : 18;
  }
}

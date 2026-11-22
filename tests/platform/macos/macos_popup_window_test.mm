#include "macos_popup_window_internal.hpp"
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

// Placement in a 1000 x 800 work area (y-down).
int test_frames() {
  const Rect work{{0, 0}, {1000, 800}};
  const cgpui::Size menu{200, 300};
  const auto below = cgpui::macos_popup_frame({{100, 100}, {50, 20}}, menu,
                                              WindowPopupSide::below, 4, work);
  if (!same(below.bounds, 100, 124, 200, 300) || below.flipped || below.shifted) return 1;
  // Near the bottom edge "below" flips above the anchor.
  const auto flipped = cgpui::macos_popup_frame({{100, 700}, {50, 20}}, menu,
                                                WindowPopupSide::below, 4, work);
  if (!same(flipped.bounds, 100, 396, 200, 300) || !flipped.flipped) return 2;
  // Near the right edge "right" flips left; near the bottom it shifts up.
  const auto left = cgpui::macos_popup_frame({{900, 650}, {60, 20}}, menu,
                                             WindowPopupSide::right, 0, work);
  if (!same(left.bounds, 700, 500, 200, 300) || !left.flipped || !left.shifted) return 3;
  // Below at the right edge shifts left to stay on screen.
  const auto shifted = cgpui::macos_popup_frame({{900, 100}, {50, 20}}, menu,
                                                WindowPopupSide::below, 0, work);
  return same(shifted.bounds, 800, 120, 200, 300) && shifted.shifted && !shifted.flipped
             ? 0
             : 4;
}

}  // namespace

int main() {
  @autoreleasepool {
    if (const int frames = test_frames(); frames != 0) return frames;
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

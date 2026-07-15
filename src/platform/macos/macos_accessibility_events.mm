#include "macos_accessibility_internal.hpp"

namespace cgpui {

std::size_t macos_accessibility_publish_live_updates(
    const std::shared_ptr<MacOSAccessibilityRegistry>& registry,
    const std::vector<PlatformAccessibilityLiveUpdate>& updates) {
  NSCAssert([NSThread isMainThread], @"Accessibility events publish on main");
  if (!registry || registry->window == nullptr) return 0;
  std::size_t published = 0;
  for (const PlatformAccessibilityLiveUpdate& update : updates) {
    CGPUIMacOSAccessibilityElement* provider =
        macos_accessibility_current_provider(registry, update.element_id);
    if (provider == nil) continue;
    NSAccessibilityNotificationName notification =
        NSAccessibilityValueChangedNotification;
    if (update.kind == PlatformAccessibilityLiveUpdateKind::focus_changed) {
      notification = NSAccessibilityFocusedUIElementChangedNotification;
    }
    NSAccessibilityPostNotification(provider, notification);
    published += 1;
  }
  return published;
}

}  // namespace cgpui

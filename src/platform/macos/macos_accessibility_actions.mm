#include "macos_accessibility_internal.hpp"
#include "macos_window_internal.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace cgpui {

void macos_accessibility_link_actions() {}

void MacOSWindow::accessibility_action(AccessibilityActionRequested action) {
  callback_(PlatformEvent{std::move(action)});
}

bool macos_accessibility_request_action(
    CGPUIMacOSAccessibilityElement* element,
    AccessibilityActionRequested action) {
  NSCAssert([NSThread isMainThread], @"Accessibility actions run on main");
  const auto node = macos_accessibility_snapshot(element);
  const auto registry = [element cgpuiRegistry];
  if (!node.has_value() || !node->enabled || !registry ||
      registry->window == nullptr) {
    return false;
  }
  action.element_id = node->element_id;
  static_cast<MacOSWindow*>(registry->window)
      ->accessibility_action(std::move(action));
  return true;
}

}  // namespace cgpui

@implementation CGPUIMacOSAccessibilityElement (Actions)
- (NSArray<NSAccessibilityActionName>*)accessibilityActionNames {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  if (!node.has_value() || !node->enabled) return @[];
  NSMutableArray* result = [[NSMutableArray alloc] initWithCapacity:2];
  if (node->patterns.invokable || node->patterns.toggled.has_value()) {
    [result addObject:NSAccessibilityPressAction];
  }
  if (node->patterns.range.has_value()) {
    [result addObject:NSAccessibilityIncrementAction];
    [result addObject:NSAccessibilityDecrementAction];
  }
  return result;
}
- (void)accessibilityPerformAction:(NSAccessibilityActionName)action {
  if ([action isEqualToString:NSAccessibilityPressAction]) {
    (void)[self accessibilityPerformPress];
  } else if ([action isEqualToString:NSAccessibilityIncrementAction]) {
    (void)[self accessibilityPerformIncrement];
  } else if ([action isEqualToString:NSAccessibilityDecrementAction]) {
    (void)[self accessibilityPerformDecrement];
  }
}
- (BOOL)accessibilityPerformPress {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  if (!node.has_value()) return NO;
  if (node->patterns.invokable) {
    return cgpui::macos_accessibility_request_action(
        self, {.kind = cgpui::AccessibilityActionKind::invoke});
  }
  if (node->patterns.toggled.has_value()) {
    return cgpui::macos_accessibility_request_action(
        self, {.kind = cgpui::AccessibilityActionKind::toggle});
  }
  return NO;
}
- (BOOL)cgpuiAdjustRangeBy:(double)direction {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  if (!node.has_value() || !node->patterns.range.has_value()) return NO;
  const auto& range = *node->patterns.range;
  const double next = std::clamp(
      range.value + direction * range.small_change,
      range.minimum,
      range.maximum);
  return cgpui::macos_accessibility_request_action(
      self,
      {.kind = cgpui::AccessibilityActionKind::set_range_value,
       .numeric_value = next});
}
- (BOOL)accessibilityPerformIncrement {
  return [self cgpuiAdjustRangeBy:1.0];
}
- (BOOL)accessibilityPerformDecrement {
  return [self cgpuiAdjustRangeBy:-1.0];
}
- (void)setAccessibilityValue:(id)value {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  if (!node.has_value()) return;
  if (node->patterns.range.has_value() &&
      [value isKindOfClass:[NSNumber class]]) {
    const auto& range = *node->patterns.range;
    const double numeric = [value doubleValue];
    if (numeric >= range.minimum && numeric <= range.maximum) {
      (void)cgpui::macos_accessibility_request_action(
          self,
          {.kind = cgpui::AccessibilityActionKind::set_range_value,
           .numeric_value = numeric});
    }
    return;
  }
  if (!node->patterns.value_settable ||
      ![value isKindOfClass:[NSString class]]) {
    return;
  }
  NSString* string = value;
  const char* bytes = [string UTF8String];
  (void)cgpui::macos_accessibility_request_action(
      self,
      {.kind = cgpui::AccessibilityActionKind::set_value,
       .value = bytes == nullptr ? std::string{} : std::string{bytes}});
}
- (BOOL)isAccessibilitySelectorAllowed:(SEL)selector {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  if (!node.has_value() || !node->enabled) return NO;
  if (selector == @selector(accessibilityPerformPress)) {
    return node->patterns.invokable || node->patterns.toggled.has_value();
  }
  if (selector == @selector(accessibilityPerformIncrement) ||
      selector == @selector(accessibilityPerformDecrement)) {
    return node->patterns.range.has_value();
  }
  if (selector == @selector(setAccessibilityValue:)) {
    return node->patterns.value_settable || node->patterns.range.has_value();
  }
  return [super isAccessibilitySelectorAllowed:selector];
}
@end

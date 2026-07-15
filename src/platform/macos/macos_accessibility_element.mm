#include "macos_accessibility_internal.hpp"
#include "macos_input_internal.hpp"

#include <algorithm>
#include <string_view>

namespace {

NSString* ns_string(std::string_view value) {
  NSString* result = [[NSString alloc] initWithBytes:value.data()
                                               length:value.size()
                                             encoding:NSUTF8StringEncoding];
  return result == nil ? @"" : result;
}

NSAccessibilityRole role_for(cgpui::PlatformAccessibilityRole role) {
  using Role = cgpui::PlatformAccessibilityRole;
  switch (role) {
    case Role::label:
    case Role::text: return NSAccessibilityStaticTextRole;
    case Role::button: return NSAccessibilityButtonRole;
    case Role::text_input: return NSAccessibilityTextFieldRole;
    case Role::image: return NSAccessibilityImageRole;
    case Role::checkbox:
    case Role::switch_control: return NSAccessibilityCheckBoxRole;
    case Role::radio: return NSAccessibilityRadioButtonRole;
    case Role::slider: return NSAccessibilitySliderRole;
    case Role::list_item: return NSAccessibilityRowRole;
    case Role::menu_item: return NSAccessibilityMenuItemRole;
    case Role::generic: return NSAccessibilityGroupRole;
  }
}

id value_for(const cgpui::PlatformAccessibilityNodeUpdate& node) {
  if (node.patterns.range.has_value()) {
    return @(node.patterns.range->value);
  }
  if (node.patterns.toggled.has_value()) {
    return @(*node.patterns.toggled);
  }
  if (!node.value.empty()) return ns_string(node.value);
  if (!node.text.empty()) return ns_string(node.text);
  return nil;
}

}  // namespace

@implementation CGPUIMacOSAccessibilityElement
- (instancetype)initWithRegistry:
                    (std::shared_ptr<cgpui::MacOSAccessibilityRegistry>)registry
                             key:(cgpui::MacOSAccessibilityProviderKey)key {
  self = [super init];
  if (self != nil) {
    registry_ = registry;
    key_ = key;
  }
  return self;
}
- (std::shared_ptr<cgpui::MacOSAccessibilityRegistry>)cgpuiRegistry {
  return registry_.lock();
}
- (cgpui::MacOSAccessibilityProviderKey)cgpuiProviderKey {
  return key_;
}
- (BOOL)isAccessibilityElement {
  return cgpui::macos_accessibility_snapshot(self).has_value();
}
- (BOOL)accessibilityElement {
  return [self isAccessibilityElement];
}
- (BOOL)isAccessibilityEnabled {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  return node.has_value() && node->enabled;
}
- (BOOL)isAccessibilityFocused {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  return node.has_value() && node->focused;
}
- (BOOL)accessibilityFocused {
  return [self isAccessibilityFocused];
}
- (NSAccessibilityRole)accessibilityRole {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  return node.has_value() ? role_for(node->role) : nil;
}
- (NSAccessibilitySubrole)accessibilitySubrole {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  return node.has_value() &&
          node->role == cgpui::PlatformAccessibilityRole::switch_control
      ? NSAccessibilitySwitchSubrole
      : nil;
}
- (NSString*)accessibilityRoleDescription {
  const auto role = [self accessibilityRole];
  const auto subrole = [self accessibilitySubrole];
  return role == nil ? nil : NSAccessibilityRoleDescription(role, subrole);
}
- (NSString*)accessibilityLabel {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  return node.has_value() ? ns_string(node->name) : nil;
}
- (id)accessibilityValue {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  return node.has_value() ? value_for(*node) : nil;
}
- (NSRect)accessibilityFrame {
  NSCAssert([NSThread isMainThread], @"Accessibility frames resolve on main");
  const auto node = cgpui::macos_accessibility_snapshot(self);
  const auto registry = [self cgpuiRegistry];
  if (!node.has_value() || !node->bounds.has_value() || !registry ||
      registry->content_view == nil) {
    return NSZeroRect;
  }
  CGPUIMacOSContentView* content_view = registry->content_view;
  const cgpui::Rect& bounds = *node->bounds;
  return NSAccessibilityFrameInView(
      content_view,
      NSMakeRect(bounds.origin.x, bounds.origin.y,
                 bounds.size.width, bounds.size.height));
}
- (NSInteger)accessibilityNumberOfCharacters {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  return node.has_value() ? [ns_string(node->text) length] : 0;
}
- (NSRange)accessibilitySelectedTextRange {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  if (!node.has_value()) return NSMakeRange(NSNotFound, 0);
  return NSMakeRange([ns_string(node->text) length], 0);
}
- (NSArray<NSValue*>*)accessibilitySelectedTextRanges {
  const NSRange range = [self accessibilitySelectedTextRange];
  return range.location == NSNotFound ? @[] : @[[NSValue valueWithRange:range]];
}
- (NSString*)accessibilitySelectedText {
  return cgpui::macos_accessibility_snapshot(self).has_value() ? @"" : nil;
}
- (NSString*)accessibilityStringForRange:(NSRange)range {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  if (!node.has_value()) return nil;
  NSString* text = ns_string(node->text);
  return NSMaxRange(range) <= [text length]
      ? [text substringWithRange:range]
      : nil;
}
@end

#include "macos_accessibility_internal.hpp"
#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"

#include <utility>

namespace cgpui {

void MacOSWindow::update_accessibility_tree(
    PlatformAccessibilityTreeUpdate update) {
  accessibility_state_.update(std::move(update));
}

void MacOSAccessibilityState::update(
    PlatformAccessibilityTreeUpdate update) {
  NSCAssert([NSThread isMainThread], @"Accessibility trees update on main");
  if (!registry_ || registry_->window == nullptr) return;

  registry_->generation += 1;
  registry_->root_element_id = update.root_element_id;
  registry_->nodes = std::move(update.nodes);
  registry_->node_indices.clear();
  registry_->children.clear();
  registry_->providers = [[NSMutableDictionary alloc] init];

  for (std::size_t index = 0; index < registry_->nodes.size(); ++index) {
    const PlatformAccessibilityNodeUpdate& node = registry_->nodes[index];
    registry_->node_indices[node.element_id] = index;
    if (node.parent_element_id.has_value()) {
      registry_->children[*node.parent_element_id].push_back(node.element_id);
    }
    auto* provider = [[CGPUIMacOSAccessibilityElement alloc]
        initWithRegistry:registry_
                     key:MacOSAccessibilityProviderKey{
                             .window_id = registry_->window_id,
                             .element_id = node.element_id,
                             .generation = registry_->generation}];
    registry_->providers[@(node.element_id)] = provider;
  }

  registry_->notification_count +=
      macos_accessibility_publish_live_updates(registry_, update.live_updates);
}

}  // namespace cgpui

@interface CGPUIMacOSAccessibilityElement (Tree)
@end

@implementation CGPUIMacOSAccessibilityElement (Tree)
- (id)accessibilityParent {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  const auto registry = [self cgpuiRegistry];
  if (!node.has_value() || !registry) return nil;
  if (!node->parent_element_id.has_value()) return registry->content_view;
  return cgpui::macos_accessibility_current_provider(
      registry, *node->parent_element_id);
}
- (NSArray*)accessibilityChildren {
  const auto node = cgpui::macos_accessibility_snapshot(self);
  const auto registry = [self cgpuiRegistry];
  if (!node.has_value() || !registry) return @[];
  const auto children = registry->children.find(node->element_id);
  if (children == registry->children.end()) return @[];
  NSMutableArray* result = [[NSMutableArray alloc]
      initWithCapacity:children->second.size()];
  for (const std::uint64_t element_id : children->second) {
    auto* provider =
        cgpui::macos_accessibility_current_provider(registry, element_id);
    if (provider != nil) [result addObject:provider];
  }
  return result;
}
- (NSArray*)accessibilityVisibleChildren {
  return [self accessibilityChildren];
}
- (NSArray*)accessibilityChildrenInNavigationOrder {
  return [self accessibilityChildren];
}
- (id)accessibilityWindow {
  const auto registry = [self cgpuiRegistry];
  return registry && registry->content_view != nil
      ? [registry->content_view window]
      : nil;
}
- (id)accessibilityTopLevelUIElement {
  return [self accessibilityWindow];
}
@end

@interface CGPUIMacOSContentView (Accessibility)
@end

@implementation CGPUIMacOSContentView (Accessibility)
- (BOOL)isAccessibilityElement {
  return NO;
}
- (NSArray*)accessibilityChildren {
  NSCAssert([NSThread isMainThread], @"Accessibility queries run on main");
  if (window_adapter_ == nullptr) return @[];
  auto* root = cgpui::macos_accessibility_root(
      *static_cast<cgpui::MacOSWindow*>(window_adapter_));
  return root == nil ? @[] : @[root];
}
- (id)accessibilityFocusedUIElement {
  NSCAssert([NSThread isMainThread], @"Accessibility queries run on main");
  if (window_adapter_ == nullptr) return nil;
  return static_cast<cgpui::MacOSWindow*>(window_adapter_)
      ->accessibility_state().focused();
}
@end

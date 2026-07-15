#pragma once

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

@class CGPUIMacOSContentView;
@class CGPUIMacOSAccessibilityElement;

namespace cgpui {

class MacOSWindow;

struct MacOSAccessibilityProviderKey {
  std::uint64_t window_id = 0;
  std::uint64_t element_id = 0;
  std::uint64_t generation = 0;

  bool operator==(const MacOSAccessibilityProviderKey&) const = default;
};

class MacOSAccessibilityRegistry;

}  // namespace cgpui

@interface CGPUIMacOSAccessibilityElement : NSAccessibilityElement {
 @private
  std::weak_ptr<cgpui::MacOSAccessibilityRegistry> registry_;
  cgpui::MacOSAccessibilityProviderKey key_;
}
- (instancetype)initWithRegistry:
                    (std::shared_ptr<cgpui::MacOSAccessibilityRegistry>)registry
                             key:(cgpui::MacOSAccessibilityProviderKey)key;
- (std::shared_ptr<cgpui::MacOSAccessibilityRegistry>)cgpuiRegistry;
- (cgpui::MacOSAccessibilityProviderKey)cgpuiProviderKey;
- (BOOL)accessibilityFocused;
@end

@interface CGPUIMacOSAccessibilityElement (Actions)
- (void)accessibilityPerformAction:(NSAccessibilityActionName)action;
@end

namespace cgpui {

struct MacOSAccessibilityDiagnostics {
  std::uint64_t generation = 0;
  std::size_t node_count = 0;
  std::size_t notification_count = 0;
  std::vector<std::string> notification_names;
};

class MacOSAccessibilityRegistry {
 public:
  std::uint64_t window_id = 0;
  std::uint64_t generation = 0;
  std::uint64_t root_element_id = 0;
  std::size_t notification_count = 0;
  std::vector<std::string> notification_names;
  void* window = nullptr;
  __weak CGPUIMacOSContentView* content_view = nil;
  std::vector<PlatformAccessibilityNodeUpdate> nodes;
  std::unordered_map<std::uint64_t, std::size_t> node_indices;
  std::unordered_map<std::uint64_t, std::vector<std::uint64_t>> children;
  __strong NSMutableDictionary<NSNumber*, CGPUIMacOSAccessibilityElement*>*
      providers = nil;
};

class MacOSAccessibilityState {
 public:
  MacOSAccessibilityState(
      MacOSWindow* window,
      CGPUIMacOSContentView* content_view);
  ~MacOSAccessibilityState();
  MacOSAccessibilityState(const MacOSAccessibilityState&) = delete;
  MacOSAccessibilityState& operator=(const MacOSAccessibilityState&) = delete;

  void update(PlatformAccessibilityTreeUpdate update);
  void detach();
  [[nodiscard]] CGPUIMacOSAccessibilityElement* root() const;
  [[nodiscard]] CGPUIMacOSAccessibilityElement* provider(
      std::uint64_t element_id) const;
  [[nodiscard]] CGPUIMacOSAccessibilityElement* focused() const;
  [[nodiscard]] MacOSAccessibilityDiagnostics diagnostics() const;

 private:
  std::shared_ptr<MacOSAccessibilityRegistry> registry_;
};

[[nodiscard]] std::optional<PlatformAccessibilityNodeUpdate>
macos_accessibility_snapshot(CGPUIMacOSAccessibilityElement* element);
[[nodiscard]] CGPUIMacOSAccessibilityElement*
macos_accessibility_current_provider(
    const std::shared_ptr<MacOSAccessibilityRegistry>& registry,
    std::uint64_t element_id);
[[nodiscard]] std::size_t macos_accessibility_publish_live_updates(
    const std::shared_ptr<MacOSAccessibilityRegistry>& registry,
    const std::vector<PlatformAccessibilityLiveUpdate>& updates);
[[nodiscard]] bool macos_accessibility_request_action(
    CGPUIMacOSAccessibilityElement* element,
    AccessibilityActionRequested action);
void macos_accessibility_link_actions();

[[nodiscard]] CGPUIMacOSAccessibilityElement* macos_accessibility_root(
    PlatformWindow& window);
[[nodiscard]] CGPUIMacOSAccessibilityElement* macos_accessibility_provider(
    PlatformWindow& window,
    std::uint64_t element_id);
[[nodiscard]] MacOSAccessibilityDiagnostics macos_accessibility_diagnostics(
    PlatformWindow& window);

}  // namespace cgpui

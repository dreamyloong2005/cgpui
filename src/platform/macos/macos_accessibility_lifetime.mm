#include "macos_accessibility_internal.hpp"
#include "macos_window_internal.hpp"

#include <atomic>

namespace cgpui {
namespace {

std::uint64_t next_window_id() {
  static std::atomic<std::uint64_t> next{1};
  return next.fetch_add(1, std::memory_order_relaxed);
}

MacOSWindow* macos_window(PlatformWindow& window) {
  return dynamic_cast<MacOSWindow*>(&window);
}

}  // namespace

MacOSAccessibilityState::MacOSAccessibilityState(
    MacOSWindow* window,
    CGPUIMacOSContentView* content_view)
    : registry_(std::make_shared<MacOSAccessibilityRegistry>()) {
  macos_accessibility_link_actions();
  registry_->window_id = next_window_id();
  registry_->window = window;
  registry_->content_view = content_view;
}

MacOSAccessibilityState::~MacOSAccessibilityState() {
  detach();
  registry_.reset();
}

void MacOSAccessibilityState::detach() {
  if (!registry_) return;
  registry_->generation += 1;
  registry_->window = nullptr;
  registry_->content_view = nil;
  registry_->providers = nil;
  registry_->nodes.clear();
  registry_->node_indices.clear();
  registry_->children.clear();
  registry_->notification_names.clear();
  registry_->root_element_id = 0;
}

CGPUIMacOSAccessibilityElement* MacOSAccessibilityState::provider(
    std::uint64_t element_id) const {
  return macos_accessibility_current_provider(registry_, element_id);
}

CGPUIMacOSAccessibilityElement* MacOSAccessibilityState::root() const {
  return registry_ ? provider(registry_->root_element_id) : nil;
}

CGPUIMacOSAccessibilityElement* MacOSAccessibilityState::focused() const {
  if (!registry_ || registry_->window == nullptr) return nil;
  for (const PlatformAccessibilityNodeUpdate& node : registry_->nodes) {
    if (node.focused) return provider(node.element_id);
  }
  return nil;
}

MacOSAccessibilityDiagnostics MacOSAccessibilityState::diagnostics() const {
  return registry_ == nullptr
      ? MacOSAccessibilityDiagnostics{}
      : MacOSAccessibilityDiagnostics{
            .generation = registry_->generation,
            .node_count = registry_->nodes.size(),
            .notification_count = registry_->notification_count,
            .notification_names = registry_->notification_names};
}

std::optional<PlatformAccessibilityNodeUpdate> macos_accessibility_snapshot(
    CGPUIMacOSAccessibilityElement* element) {
  if (element == nil) return std::nullopt;
  const auto registry = [element cgpuiRegistry];
  const MacOSAccessibilityProviderKey key = [element cgpuiProviderKey];
  if (!registry || registry->window == nullptr ||
      key.window_id != registry->window_id ||
      key.generation != registry->generation) {
    return std::nullopt;
  }
  const auto index = registry->node_indices.find(key.element_id);
  if (index == registry->node_indices.end()) return std::nullopt;
  return registry->nodes[index->second];
}

CGPUIMacOSAccessibilityElement* macos_accessibility_current_provider(
    const std::shared_ptr<MacOSAccessibilityRegistry>& registry,
    std::uint64_t element_id) {
  if (!registry || registry->window == nullptr || registry->providers == nil) {
    return nil;
  }
  return registry->providers[@(element_id)];
}

CGPUIMacOSAccessibilityElement* macos_accessibility_root(
    PlatformWindow& window) {
  MacOSWindow* native = macos_window(window);
  return native == nullptr ? nil : native->accessibility_state().root();
}

CGPUIMacOSAccessibilityElement* macos_accessibility_provider(
    PlatformWindow& window,
    std::uint64_t element_id) {
  MacOSWindow* native = macos_window(window);
  return native == nullptr
      ? nil
      : native->accessibility_state().provider(element_id);
}

MacOSAccessibilityDiagnostics macos_accessibility_diagnostics(
    PlatformWindow& window) {
  MacOSWindow* native = macos_window(window);
  return native == nullptr
      ? MacOSAccessibilityDiagnostics{}
      : native->accessibility_state().diagnostics();
}

}  // namespace cgpui

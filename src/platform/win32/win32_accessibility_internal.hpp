#pragma once

#include "cgpui/platform/platform.hpp"
#include "win32_uia_navigation_internal.hpp"
#include "win32_uia_provider_internal.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace cgpui {

class Win32UiaAccessibilityAdapter {
 public:
  Win32UiaAccessibilityAdapter() = default;
  ~Win32UiaAccessibilityAdapter();
  Win32UiaAccessibilityAdapter(const Win32UiaAccessibilityAdapter&) = delete;
  Win32UiaAccessibilityAdapter& operator=(
      const Win32UiaAccessibilityAdapter&) = delete;

  void attach(HWND hwnd);
  void detach();
  void update(PlatformAccessibilityTreeUpdate update);
  bool handle_get_object(WPARAM wparam, LPARAM lparam, LRESULT& result) const;

  [[nodiscard]] std::uint64_t root_element_id() const;
  [[nodiscard]] std::size_t node_count() const;
  [[nodiscard]] std::size_t focused_node_count() const;
  [[nodiscard]] std::size_t text_input_node_count() const;
  [[nodiscard]] const std::vector<Win32UiaProviderNode>& uia_provider_nodes()
      const;
  [[nodiscard]] const std::vector<PlatformAccessibilityLiveUpdate>&
  last_live_updates() const;
  [[nodiscard]] IRawElementProviderSimple* root_provider() const;
  [[nodiscard]] IRawElementProviderSimple* provider_for_element(
      std::uint64_t element_id) const;

 private:
  void release_providers();
  PlatformAccessibilityTreeUpdate last_update_;
  std::vector<Win32UiaProviderNode> provider_nodes_;
  std::vector<IRawElementProviderSimple*> providers_;
  Win32UiaProviderTreeHandle provider_tree_;
  std::vector<PlatformAccessibilityLiveUpdate> live_updates_;
  HWND hwnd_ = nullptr;
  std::uint64_t root_element_id_ = 0;
  std::size_t node_count_ = 0;
  std::size_t focused_node_count_ = 0;
  std::size_t text_input_node_count_ = 0;
};

} // namespace cgpui

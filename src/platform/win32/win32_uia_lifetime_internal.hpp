#pragma once

#include "win32_uia_navigation_internal.hpp"

#include <functional>
#include <vector>

namespace cgpui {

void reconcile_win32_uia_provider_lifetime(
    Win32UiaProviderTreeHandle& tree,
    HWND hwnd,
    std::uint64_t root_element_id,
    const std::vector<Win32UiaProviderNode>& previous_nodes,
    const std::vector<Win32UiaProviderNode>& current_nodes,
    std::function<void(AccessibilityActionRequested)> action_callback,
    std::vector<IRawElementProviderSimple*>& providers);
void retire_win32_uia_provider_lifetime(
    Win32UiaProviderTreeHandle& tree,
    std::vector<IRawElementProviderSimple*>& providers);

} // namespace cgpui

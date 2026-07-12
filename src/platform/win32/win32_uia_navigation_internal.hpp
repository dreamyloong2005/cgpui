#pragma once

#include "win32_uia_provider_internal.hpp"

#include <vector>

namespace cgpui {

Win32UiaProviderTreeHandle create_win32_uia_provider_tree(
    HWND hwnd,
    std::uint64_t root_element_id,
    std::vector<Win32UiaProviderNode> nodes);
void set_win32_uia_provider_tree_hwnd(
    const Win32UiaProviderTreeHandle& tree,
    HWND hwnd);
HWND win32_uia_provider_tree_hwnd(
    const Win32UiaProviderTreeHandle& tree);
void register_win32_uia_fragment(
    const Win32UiaProviderTreeHandle& tree,
    std::uint64_t element_id,
    IRawElementProviderFragment* provider);
void unregister_win32_uia_fragment(
    const Win32UiaProviderTreeHandle& tree,
    std::uint64_t element_id,
    IRawElementProviderFragment* provider);
HRESULT navigate_win32_uia_fragment(
    const Win32UiaProviderTreeHandle& tree,
    std::uint64_t element_id,
    NavigateDirection direction,
    IRawElementProviderFragment** result);
HRESULT get_win32_uia_fragment_root(
    const Win32UiaProviderTreeHandle& tree,
    IRawElementProviderFragmentRoot** result);
HRESULT get_win32_uia_focus(
    const Win32UiaProviderTreeHandle& tree,
    IRawElementProviderFragment** result);
HRESULT get_win32_uia_provider_from_point(
    const Win32UiaProviderTreeHandle& tree,
    double x,
    double y,
    IRawElementProviderFragment** result);

} // namespace cgpui

#include "win32_internal.hpp"

#include <utility>

namespace cgpui {
namespace {

bool append_native_menu_item(
    HMENU parent,
    const NativeMenuItem& item,
    UINT_PTR command_id,
    HMENU submenu = nullptr) {
  MENUITEMINFOW info{.cbSize = sizeof(MENUITEMINFOW)};
  if (item.kind == NativeMenuItemKind::separator) {
    info.fMask = MIIM_FTYPE;
    info.fType = MFT_SEPARATOR;
  } else {
    std::wstring title = widen(item.title);
    info.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_STRING;
    info.fType = MFT_STRING | (item.radio ? MFT_RADIOCHECK : 0U);
    info.fState = item.enabled ? MFS_ENABLED : MFS_GRAYED;
    if (item.checked) info.fState |= MFS_CHECKED;
    info.dwTypeData = title.data();
    info.cch = static_cast<UINT>(title.size());
    if (submenu != nullptr) {
      info.fMask |= MIIM_SUBMENU;
      info.hSubMenu = submenu;
    } else {
      info.fMask |= MIIM_ID;
      info.wID = static_cast<UINT>(command_id);
    }
    return InsertMenuItemW(
               parent,
               static_cast<UINT>(GetMenuItemCount(parent)),
               TRUE,
               &info) != FALSE;
  }
  return InsertMenuItemW(
             parent,
             static_cast<UINT>(GetMenuItemCount(parent)),
             TRUE,
             &info) != FALSE;
}

bool append_menu_items(
    HMENU parent,
    const std::vector<NativeMenuItem>& items,
    UINT_PTR& next_command_id) {
  for (const NativeMenuItem& item : items) {
    if (item.kind == NativeMenuItemKind::separator) {
      if (!append_native_menu_item(parent, item, 0)) return false;
      continue;
    }

    if (item.kind == NativeMenuItemKind::submenu) {
      HMENU submenu = CreatePopupMenu();
      if (submenu == nullptr) return false;
      if (!append_menu_items(submenu, item.children, next_command_id) ||
          !append_native_menu_item(parent, item, 0, submenu)) {
        DestroyMenu(submenu);
        return false;
      }
      continue;
    }

    if (!append_native_menu_item(parent, item, next_command_id++)) {
      return false;
    }
  }
  return true;
}

} // namespace

Win32NativeMenuTree::Win32NativeMenuTree(HMENU root) : root_(root) {}

Win32NativeMenuTree::~Win32NativeMenuTree() {
  if (root_ != nullptr) DestroyMenu(root_);
}

Win32NativeMenuTree::Win32NativeMenuTree(
    Win32NativeMenuTree&& other) noexcept
    : root_(std::exchange(other.root_, nullptr)) {}

Win32NativeMenuTree& Win32NativeMenuTree::operator=(
    Win32NativeMenuTree&& other) noexcept {
  if (this == &other) return *this;
  if (root_ != nullptr) DestroyMenu(root_);
  root_ = std::exchange(other.root_, nullptr);
  return *this;
}

std::optional<Win32NativeMenuTree> Win32NativeMenuTree::build(
    const NativeMenuModel& model) {
  if (model.items.empty()) return Win32NativeMenuTree{};
  HMENU root = CreateMenu();
  if (root == nullptr) return std::nullopt;
  UINT_PTR next_command_id = 0x1000;
  if (!append_menu_items(root, model.items, next_command_id)) {
    DestroyMenu(root);
    return std::nullopt;
  }
  return Win32NativeMenuTree(root);
}

HMENU Win32NativeMenuTree::root() const { return root_; }

bool win32_apply_native_menu(HWND hwnd, HMENU menu) {
  if (hwnd == nullptr || SetMenu(hwnd, menu) == FALSE) return false;
  return DrawMenuBar(hwnd) != FALSE;
}

} // namespace cgpui

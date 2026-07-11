#include "win32_internal.hpp"

#include <utility>

namespace cgpui {
namespace {

bool append_menu_items(
    HMENU parent,
    const std::vector<NativeMenuItem>& items,
    UINT_PTR& next_command_id) {
  for (const NativeMenuItem& item : items) {
    if (item.kind == NativeMenuItemKind::separator) {
      if (AppendMenuW(parent, MF_SEPARATOR, 0, nullptr) == FALSE) return false;
      continue;
    }

    const std::wstring title = widen(item.title);
    if (item.kind == NativeMenuItemKind::submenu) {
      HMENU submenu = CreatePopupMenu();
      if (submenu == nullptr) return false;
      if (!append_menu_items(submenu, item.children, next_command_id) ||
          AppendMenuW(
              parent,
              MF_POPUP | MF_STRING,
              reinterpret_cast<UINT_PTR>(submenu),
              title.c_str()) == FALSE) {
        DestroyMenu(submenu);
        return false;
      }
      continue;
    }

    if (AppendMenuW(parent, MF_STRING, next_command_id++, title.c_str()) ==
        FALSE) {
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

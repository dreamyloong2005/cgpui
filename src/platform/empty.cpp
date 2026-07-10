#include "cgpui/platform/platform.hpp"

namespace cgpui {
namespace {

std::size_t native_menu_item_count(const std::vector<NativeMenuItem>& items) {
  std::size_t count = 0;
  for (const NativeMenuItem& item : items) {
    count += 1;
    count += native_menu_item_count(item.children);
  }
  return count;
}

std::size_t native_menu_accelerator_count(
    const std::vector<NativeMenuItem>& items) {
  std::size_t count = 0;
  for (const NativeMenuItem& item : items) {
    if (item.accelerator.has_value()) {
      count += 1;
    }
    count += native_menu_accelerator_count(item.children);
  }
  return count;
}

} // namespace

std::size_t native_menu_item_count(const NativeMenuModel& model) {
  return native_menu_item_count(model.items);
}

std::size_t native_menu_accelerator_count(const NativeMenuModel& model) {
  return native_menu_accelerator_count(model.items);
}

void PlatformWindow::update_accessibility_tree(
    PlatformAccessibilityTreeUpdate update) {
  (void)update;
}

void PlatformApplication::request_wakeup() {}

PlatformMenuInstallationResult PlatformApplication::install_native_menu(
    NativeMenuModel menu) {
  return PlatformMenuInstallationResult{
      .supported = false,
      .backend = "unsupported",
      .menu_count = menu.items.size(),
      .item_count = native_menu_item_count(menu),
      .accelerator_count = native_menu_accelerator_count(menu),
  };
}

NativeFileDialogResult PlatformApplication::show_native_file_dialog(
    NativeFileDialogOptions options) {
  return NativeFileDialogResult{
      .supported = false,
      .accepted = false,
      .backend = "unsupported",
      .kind = options.kind,
      .filter_count = options.filters.size(),
  };
}

void cgpui_platform_anchor() {}
} // namespace cgpui

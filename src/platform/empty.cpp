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
  const std::size_t accelerator_count = native_menu_accelerator_count(menu);
  return PlatformMenuInstallationResult{
      .supported = false,
      .backend = "unsupported",
      .menu_count = menu.items.size(),
      .item_count = native_menu_item_count(menu),
      .accelerator_count = accelerator_count,
      .registered_accelerator_count = 0,
      .skipped_accelerator_count = accelerator_count,
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

NativeMessageDialogResult PlatformApplication::show_native_message_dialog(
    NativeMessageDialogOptions) {
  return NativeMessageDialogResult{
      .supported = false,
      .backend = "unsupported",
      .error_message = "native message dialog unsupported",
  };
}

PlatformOpenUrlResult PlatformApplication::open_url(std::string) {
  return PlatformOpenUrlResult{
      .supported = false,
      .backend = "unsupported",
      .error_message = "open URL unsupported",
  };
}

void PlatformApplication::set_reopen_callback(PlatformReopenCallback callback) {
  reopen_callback_ = std::move(callback);
}

PlatformReopenResult PlatformApplication::request_reopen() {
  return PlatformReopenResult{
      .supported = false,
      .backend = "unsupported",
      .error_message = "reopen unsupported",
  };
}

PlatformReopenResult PlatformApplication::dispatch_reopen(std::string backend) {
  PlatformReopenResult result{.supported = true, .backend = std::move(backend)};
  if (!reopen_callback_) {
    result.error_message = "reopen callback not registered";
    return result;
  }
  reopen_callback_();
  result.requested = true;
  return result;
}

void cgpui_platform_anchor() {}
} // namespace cgpui

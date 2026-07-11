#include "wayland_internal.hpp"

namespace cgpui {

class WaylandNativeMenuState {
 public:
  PlatformMenuInstallationResult install_native_menu(NativeMenuModel menu) {
    model_ = std::move(menu);
    const std::size_t accelerator_count = native_menu_accelerator_count(model_);
    last_menu_installation_ = PlatformMenuInstallationResult{
        .supported = false,
        .backend = "wayland",
        .menu_count = model_.items.size(),
        .item_count = native_menu_item_count(model_),
        .accelerator_count = accelerator_count,
        .registered_accelerator_count = 0,
        .skipped_accelerator_count = accelerator_count,
    };
    return last_menu_installation_;
  }

  [[nodiscard]] const PlatformMenuInstallationResult& last_menu_installation()
      const {
    return last_menu_installation_;
  }

 private:
  NativeMenuModel model_;
  PlatformMenuInstallationResult last_menu_installation_;
};

class WaylandNativeFileDialogState {
 public:
  NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) {
    options_ = std::move(options);
    last_file_dialog_result_ = NativeFileDialogResult{
        .supported = false,
        .accepted = false,
        .backend = "wayland",
        .kind = options_.kind,
        .filter_count = options_.filters.size(),
    };
    return last_file_dialog_result_;
  }

  [[nodiscard]] const NativeFileDialogResult& last_file_dialog_result() const {
    return last_file_dialog_result_;
  }

 private:
  NativeFileDialogOptions options_;
  NativeFileDialogResult last_file_dialog_result_;
};



void WaylandNativeMenuStateDeleter::operator()(WaylandNativeMenuState* state) const {
  delete state;
}

WaylandNativeMenuStatePtr create_wayland_native_menu_state() {
  return WaylandNativeMenuStatePtr(new WaylandNativeMenuState());
}

PlatformMenuInstallationResult wayland_install_native_menu(
    WaylandNativeMenuState& state,
    NativeMenuModel menu) {
  return state.install_native_menu(std::move(menu));
}

void WaylandNativeFileDialogStateDeleter::operator()(
    WaylandNativeFileDialogState* state) const {
  delete state;
}

WaylandNativeFileDialogStatePtr create_wayland_native_file_dialog_state() {
  return WaylandNativeFileDialogStatePtr(new WaylandNativeFileDialogState());
}

NativeFileDialogResult wayland_show_native_file_dialog(
    WaylandNativeFileDialogState& state,
    NativeFileDialogOptions options) {
  return state.show_native_file_dialog(std::move(options));
}

} // namespace cgpui

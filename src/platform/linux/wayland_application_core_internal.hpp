#pragma once

#include "wayland_internal.hpp"
#include "wayland_cursor_theme_internal.hpp"
#include "wayland_pointer_scroll_frame_internal.hpp"

namespace cgpui {

class WaylandApplication final : public PlatformApplication {
 public:
  WaylandApplication();
  ~WaylandApplication() override;

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override;
  Result<std::unique_ptr<PlatformWindow>> create_child_window(
      const WindowDescriptor& descriptor,
      PlatformWindow& parent,
      PlatformEventCallback callback) override;
  int run() override;
  void request_wakeup() override;
  void quit() override;
  PlatformMenuInstallationResult install_native_menu(
      NativeMenuModel menu) override;
  NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) override;
  NativeMessageDialogResult show_native_message_dialog(
      NativeMessageDialogOptions options) override;
  [[nodiscard]] PlatformFontDiscoveryResult discover_font_discovery()
      const override;

 private:
#include "wayland_application_window_creation_internal.hpp"
#include "wayland_application_registry_internal.hpp"
#include "wayland_application_input_internal.hpp"
#include "wayland_application_cursor_internal.hpp"

  wl_display* display_ = nullptr;
  wl_registry* registry_ = nullptr;
  wl_compositor* compositor_ = nullptr;
  wl_shm* shm_ = nullptr;
  xdg_wm_base* shell_ = nullptr;
  zxdg_decoration_manager_v1* decoration_manager_ = nullptr;
  wl_seat* seat_ = nullptr;
  WaylandOutputScaleRegistry output_scales_;
  wl_data_device_manager* data_device_manager_ = nullptr;
  zwp_text_input_manager_v3* text_input_manager_ = nullptr;
  wp_fractional_scale_manager_v1* fractional_scale_manager_ = nullptr;
  wp_viewporter* viewporter_ = nullptr;
  wl_pointer* pointer_ = nullptr;
  wl_keyboard* keyboard_ = nullptr;
  WaylandDataDevicePtr data_device_;
  WaylandTextInputPtr text_input_;
  WaylandNativeMenuStatePtr native_menu_state_;
  PlatformMenuInstallationResult last_menu_installation_;
  WaylandNativeFileDialogStatePtr native_file_dialog_state_;
  NativeFileDialogResult last_file_dialog_result_;
  WaylandKeyboardState keyboard_state_;
  std::vector<WaylandWindow*> windows_;
  WaylandWindow* pointer_window_ = nullptr;
  WaylandWindow* keyboard_window_ = nullptr;
  int wakeup_pipe_[2] = {-1, -1};
  Point pointer_position_{};
  WaylandPointerScrollFrameState pointer_scroll_frame_;
  std::string initialization_error_;
  std::atomic_bool running_{true};
  WaylandCursorThemeLoadStatus cursor_theme_status_ =
      WaylandCursorThemeLoadStatus::unavailable;
  WaylandCursorThemeState cursor_theme_state_;
  WaylandCursorThemeResourcesPtr cursor_theme_;
  std::uint32_t pointer_enter_serial_ = 0;
};

} // namespace cgpui

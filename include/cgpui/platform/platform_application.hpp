#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/core/window.hpp"
#include "cgpui/platform/platform_appearance.hpp"
#include "cgpui/platform/platform_dpi_awareness.hpp"
#include "cgpui/platform/platform_file_dialog.hpp"
#include "cgpui/platform/platform_font_discovery.hpp"
#include "cgpui/platform/platform_native_menu.hpp"
#include "cgpui/platform/platform_message_dialog.hpp"
#include "cgpui/platform/platform_open_url.hpp"
#include "cgpui/platform/platform_lifecycle.hpp"
#include "cgpui/platform/platform_window.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace cgpui {

class PlatformApplication {
 public:
  virtual ~PlatformApplication() = default;

  virtual Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) = 0;
  virtual Result<std::unique_ptr<PlatformWindow>> create_child_window(
      const WindowDescriptor& descriptor,
      PlatformWindow& parent,
      PlatformEventCallback callback);
  [[nodiscard]] virtual PlatformFontDiscoveryResult discover_font_discovery()
      const;
  [[nodiscard]] virtual std::vector<FontFaceDescriptor> discover_font_records()
      const;
  [[nodiscard]] virtual FontDatabase discover_fonts() const;
  [[nodiscard]] virtual std::uint64_t monotonic_time_ms() const;
  virtual void request_wakeup();
  virtual void request_wakeup_after(std::uint64_t delay_ms);
  virtual void cancel_wakeup_after();
  virtual PlatformMenuInstallationResult install_native_menu(
      NativeMenuModel menu);
  virtual NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options);
  virtual NativeMessageDialogResult show_native_message_dialog(
      NativeMessageDialogOptions options);
  virtual PlatformOpenUrlResult open_url(std::string url);
  virtual void set_reopen_callback(PlatformReopenCallback callback);
  virtual PlatformReopenResult request_reopen();
  [[nodiscard]] virtual SystemAppearance system_appearance() const;
  virtual void set_system_appearance_callback(
      SystemAppearanceCallback callback);
  [[nodiscard]] virtual PlatformDpiAwarenessState dpi_awareness() const;
  // create_child_window accepts WindowKind::popup (macOS, Win32).
  // Native-window capability only; the runtime also checks its renderer.
  [[nodiscard]] virtual bool popup_windows_supported() const;

  virtual int run() = 0;
  virtual void quit() = 0;

 protected:
  [[nodiscard]] PlatformReopenResult dispatch_reopen(std::string backend);

 private:
  PlatformReopenCallback reopen_callback_;
};

Result<std::unique_ptr<PlatformApplication>> create_platform_application();

} // namespace cgpui

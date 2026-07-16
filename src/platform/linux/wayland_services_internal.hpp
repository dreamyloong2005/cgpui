#pragma once

#include "cgpui/platform/platform.hpp"
#include "linux_atspi_api_internal.hpp"
#include "wayland_protocol_internal.hpp"
#include "wayland_window_api.hpp"

#include <wayland-client.h>

#include <functional>
#include <memory>

namespace cgpui {

class WaylandTextInput;
class WaylandDataDevice;
class WaylandNativeMenuState;
class WaylandNativeFileDialogState;

struct WaylandTextInputDeleter {
  void operator()(WaylandTextInput* text_input) const;
};
using WaylandTextInputPtr =
    std::unique_ptr<WaylandTextInput, WaylandTextInputDeleter>;
WaylandTextInputPtr create_wayland_text_input();
void wayland_text_input_set_display(
    WaylandTextInput& text_input,
    wl_display* display);
void wayland_text_input_set_manager(
    WaylandTextInput& text_input,
    zwp_text_input_manager_v3* manager);
void wayland_text_input_set_window_lookup(
    WaylandTextInput& text_input,
    std::function<WaylandWindow*(wl_surface*)> lookup);
void wayland_text_input_set_modifiers_provider(
    WaylandTextInput& text_input,
    std::function<KeyboardModifiers()> provider);
void wayland_text_input_bind_to_seat(
    WaylandTextInput& text_input,
    wl_seat* seat);
void wayland_text_input_reset(WaylandTextInput& text_input);
[[nodiscard]] bool wayland_text_input_available(
    const WaylandTextInput& text_input);
void wayland_text_input_apply_placement(
    WaylandTextInput& text_input,
    WaylandWindow& window);

struct WaylandDataDeviceDeleter {
  void operator()(WaylandDataDevice* data_device) const;
};
using WaylandDataDevicePtr =
    std::unique_ptr<WaylandDataDevice, WaylandDataDeviceDeleter>;
WaylandDataDevicePtr create_wayland_data_device();
void wayland_data_device_set_display(
    WaylandDataDevice& data_device,
    wl_display* display);
void wayland_data_device_set_manager(
    WaylandDataDevice& data_device,
    wl_data_device_manager* manager);
void wayland_data_device_set_window_lookup(
    WaylandDataDevice& data_device,
    std::function<WaylandWindow*(wl_surface*)> lookup);
void wayland_data_device_bind_to_seat(
    WaylandDataDevice& data_device,
    wl_seat* seat);
void wayland_data_device_reset(WaylandDataDevice& data_device);

struct WaylandNativeMenuStateDeleter {
  void operator()(WaylandNativeMenuState* state) const;
};
using WaylandNativeMenuStatePtr =
    std::unique_ptr<WaylandNativeMenuState, WaylandNativeMenuStateDeleter>;
WaylandNativeMenuStatePtr create_wayland_native_menu_state();
PlatformMenuInstallationResult wayland_install_native_menu(
    WaylandNativeMenuState& state,
    NativeMenuModel menu);

struct WaylandNativeFileDialogStateDeleter {
  void operator()(WaylandNativeFileDialogState* state) const;
};
using WaylandNativeFileDialogStatePtr = std::unique_ptr<
    WaylandNativeFileDialogState,
    WaylandNativeFileDialogStateDeleter>;
WaylandNativeFileDialogStatePtr create_wayland_native_file_dialog_state();
NativeFileDialogResult wayland_show_native_file_dialog(
    WaylandNativeFileDialogState& state,
    NativeFileDialogOptions options);

} // namespace cgpui

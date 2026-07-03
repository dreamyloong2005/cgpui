#pragma once

#include "wayland_internal.hpp"

namespace cgpui {

class WaylandDataDevice {
 public:
  using WindowLookup = std::function<WaylandWindow*(wl_surface*)>;

  void set_display(wl_display* display);
  void set_manager(wl_data_device_manager* manager);
  void set_window_lookup(WindowLookup lookup);
  void bind_to_seat(wl_seat* seat);
  void reset_device();

 private:
  struct Offer {
    wl_data_offer* offer = nullptr;
    std::vector<std::string> mime_types;
    std::uint32_t source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    std::uint32_t selected_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    bool finished = false;
  };

  static void handle_data_offer(
      void* data,
      wl_data_device* data_device,
      wl_data_offer* offer);
  static void handle_offer_mime_type(
      void* data,
      wl_data_offer*,
      const char* mime_type);
  static void handle_offer_source_actions(
      void* data,
      wl_data_offer*,
      std::uint32_t source_actions);
  static void handle_offer_action(
      void* data,
      wl_data_offer*,
      std::uint32_t action);
  static void handle_enter(
      void* data,
      wl_data_device* data_device,
      std::uint32_t serial,
      wl_surface* surface,
      wl_fixed_t x,
      wl_fixed_t y,
      wl_data_offer* offer);
  static void handle_leave(void* data, wl_data_device* data_device);
  static void handle_motion(
      void* data,
      wl_data_device* data_device,
      std::uint32_t time,
      wl_fixed_t x,
      wl_fixed_t y);
  static void handle_drop(void* data, wl_data_device* data_device);
  static void handle_selection(
      void* data,
      wl_data_device* data_device,
      wl_data_offer* offer);

  void replace_active_offer(wl_data_offer* offer);
  void clear_active_offer();
  void clear_offer(std::unique_ptr<Offer>& offer);
  [[nodiscard]] static std::uint32_t drag_supported_actions();
  [[nodiscard]] static DragDropAction drag_action_from_wayland(
      std::uint32_t action);
  [[nodiscard]] static std::uint32_t data_offer_version(wl_data_offer* offer);
  [[nodiscard]] std::uint32_t active_offer_client_actions() const;
  [[nodiscard]] std::uint32_t preferred_drag_action() const;
  [[nodiscard]] std::optional<std::string> preferred_drag_mime_type() const;
  void negotiate_active_offer(std::uint32_t serial);
  void finish_active_offer();
  [[nodiscard]] DragDropAction current_drag_action() const;
  [[nodiscard]] std::optional<std::string> read_offer_payload(
      const std::string& mime_type);
  [[nodiscard]] bool active_offer_has_mime(std::string_view mime_type) const;
  [[nodiscard]] DragDropPayload payload_from_active_offer();

  wl_display* display_ = nullptr;
  wl_data_device_manager* manager_ = nullptr;
  wl_data_device* data_device_ = nullptr;
  std::unique_ptr<Offer> pending_offer_;
  std::unique_ptr<Offer> active_offer_;
  WaylandWindow* drag_window_ = nullptr;
  Point last_drag_position_{};
  WindowLookup find_window_;
};

} // namespace cgpui

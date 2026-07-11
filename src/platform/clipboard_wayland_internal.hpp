#pragma once

#include "clipboard_internal.hpp"

#if defined(__linux__)
#include <wayland-client.h>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#endif

namespace cgpui {

#if defined(__linux__)
[[nodiscard]] int wayland_clipboard_text_mime_rank(
    std::string_view mime_type);

struct WaylandClipboard::Connection {
  struct Offer {
    wl_data_offer* offer = nullptr;
    std::vector<std::string> mime_types;
  };

  static std::unique_ptr<Connection> create(std::string_view display_name);
  ~Connection();

  [[nodiscard]] WaylandClipboardSupport support() const;
  [[nodiscard]] std::optional<std::string> read_text();
  [[nodiscard]] bool write_text(std::string_view text);

 private:
  explicit Connection(std::string_view display_name);

  void initialize();
  static void handle_global(
      void* data,
      wl_registry* registry,
      std::uint32_t name,
      const char* interface,
      std::uint32_t version);
  static void handle_global_remove(void*, wl_registry*, std::uint32_t);

  static void handle_data_offer(
      void* data,
      wl_data_device*,
      wl_data_offer* offer);
  static void handle_offer_mime_type(
      void* data,
      wl_data_offer*,
      const char* mime_type);
  static void handle_offer_source_actions(void*, wl_data_offer*, std::uint32_t);
  static void handle_offer_action(void*, wl_data_offer*, std::uint32_t);
  static void handle_enter(
      void*,
      wl_data_device*,
      std::uint32_t,
      wl_surface*,
      wl_fixed_t,
      wl_fixed_t,
      wl_data_offer*);
  static void handle_leave(void*, wl_data_device*);
  static void handle_motion(
      void*,
      wl_data_device*,
      std::uint32_t,
      wl_fixed_t,
      wl_fixed_t);
  static void handle_drop(void*, wl_data_device*);
  static void handle_selection(
      void* data,
      wl_data_device*,
      wl_data_offer* offer);

  void clear_offer(std::unique_ptr<Offer>& offer);
  void clear_owned_source();
  static bool accepts_text_mime_type(const char* mime_type);
  static void handle_source_target(void*, wl_data_source*, const char*);
  static void handle_source_send(
      void* data,
      wl_data_source* source,
      const char* mime_type,
      std::int32_t fd);
  static void handle_source_cancelled(void* data, wl_data_source* source);
  static void handle_source_drop_performed(void*, wl_data_source*);
  static void handle_source_dnd_finished(void*, wl_data_source*);
  static void handle_source_action(void*, wl_data_source*, std::uint32_t);
  static void write_payload_to_fd(const std::string& payload, int fd);

  void start_dispatch_thread();
  void stop_dispatch_thread();
  void dispatch_owned_selection_events();
  [[nodiscard]] std::optional<std::string> preferred_text_mime_type() const;
  [[nodiscard]] std::optional<std::string> read_offer_payload(
      const std::string& mime_type);

  std::string display_name_;
  wl_display* display_ = nullptr;
  wl_registry* registry_ = nullptr;
  wl_data_device_manager* manager_ = nullptr;
  wl_seat* seat_ = nullptr;
  wl_data_device* data_device_ = nullptr;
  std::unique_ptr<Offer> pending_offer_;
  std::unique_ptr<Offer> selection_offer_;
  wl_data_source* owned_source_ = nullptr;
  std::string owned_text_;
  mutable std::mutex display_mutex_;
  std::mutex owned_selection_mutex_;
  std::atomic_bool dispatch_running_{false};
  std::thread dispatch_thread_;
  WaylandClipboardSupport support_ = WaylandClipboardSupport::unsupported;
};
#endif

} // namespace cgpui

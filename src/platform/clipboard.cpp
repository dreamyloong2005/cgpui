#include "cgpui/platform/clipboard.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(__linux__)
#include <wayland-client.h>
#endif

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <cwchar>
#include <cerrno>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#if defined(__linux__)
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#endif

namespace cgpui {
namespace {

#if defined(_WIN32)
std::wstring widen_clipboard_text(std::string_view value) {
  if (value.empty()) {
    return {};
  }

  const auto required = MultiByteToWideChar(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      nullptr,
      0);
  if (required <= 0) {
    return {};
  }

  std::wstring result(static_cast<std::size_t>(required), L'\0');
  MultiByteToWideChar(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      result.data(),
      required);
  return result;
}

std::string narrow_clipboard_text(std::wstring_view value) {
  if (value.empty()) {
    return {};
  }

  const auto required = WideCharToMultiByte(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      nullptr,
      0,
      nullptr,
      nullptr);
  if (required <= 0) {
    return {};
  }

  std::string result(static_cast<std::size_t>(required), '\0');
  WideCharToMultiByte(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      result.data(),
      required,
      nullptr,
      nullptr);
  return result;
}

class Win32Clipboard final : public Clipboard {
 public:
  [[nodiscard]] std::optional<std::string> read_text() const override {
    if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE ||
        OpenClipboard(nullptr) == FALSE) {
      return std::nullopt;
    }

    const HANDLE handle = GetClipboardData(CF_UNICODETEXT);
    if (handle == nullptr) {
      CloseClipboard();
      return std::nullopt;
    }

    const auto* text = static_cast<const wchar_t*>(GlobalLock(handle));
    if (text == nullptr) {
      CloseClipboard();
      return std::nullopt;
    }

    auto result = narrow_clipboard_text(
        std::wstring_view(text, std::wcslen(text)));
    GlobalUnlock(handle);
    CloseClipboard();
    return result;
  }

  [[nodiscard]] bool write_text(std::string_view text) override {
    const std::wstring wide_text = widen_clipboard_text(text);
    if (!text.empty() && wide_text.empty()) {
      return false;
    }

    const std::size_t byte_size =
        (wide_text.size() + 1U) * sizeof(wchar_t);
    HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, byte_size);
    if (handle == nullptr) {
      return false;
    }

    void* locked = GlobalLock(handle);
    if (locked == nullptr) {
      GlobalFree(handle);
      return false;
    }

    std::memcpy(locked, wide_text.c_str(), byte_size);
    GlobalUnlock(handle);

    if (OpenClipboard(nullptr) == FALSE) {
      GlobalFree(handle);
      return false;
    }

    if (EmptyClipboard() == FALSE) {
      CloseClipboard();
      GlobalFree(handle);
      return false;
    }

    if (SetClipboardData(CF_UNICODETEXT, handle) == nullptr) {
      CloseClipboard();
      GlobalFree(handle);
      return false;
    }

    CloseClipboard();
    return true;
  }
};
#endif

} // namespace

#if defined(__linux__)
struct WaylandClipboard::Connection {
  struct Offer {
    wl_data_offer* offer = nullptr;
    std::vector<std::string> mime_types;
  };

  static std::unique_ptr<Connection> create(std::string_view display_name) {
    auto connection = std::unique_ptr<Connection>(new Connection(display_name));
    connection->initialize();
    if (connection->support_ == WaylandClipboardSupport::unsupported) {
      return nullptr;
    }
    return connection;
  }

  ~Connection() {
    stop_dispatch_thread();
    clear_owned_source();
    clear_offer(selection_offer_);
    clear_offer(pending_offer_);
    if (data_device_ != nullptr) {
      wl_data_device_destroy(data_device_);
    }
    if (manager_ != nullptr) {
      wl_data_device_manager_destroy(manager_);
    }
    if (seat_ != nullptr) {
      wl_seat_destroy(seat_);
    }
    if (registry_ != nullptr) {
      wl_registry_destroy(registry_);
    }
    if (display_ != nullptr) {
      wl_display_disconnect(display_);
    }
  }

  [[nodiscard]] WaylandClipboardSupport support() const {
    return support_;
  }

  [[nodiscard]] std::optional<std::string> read_text() {
    if (support_ != WaylandClipboardSupport::available || display_ == nullptr) {
      return std::nullopt;
    }

    std::lock_guard display_lock(display_mutex_);
    if (wl_display_roundtrip(display_) == -1) {
      return std::nullopt;
    }

    const auto mime_type = preferred_text_mime_type();
    if (!mime_type.has_value()) {
      return std::nullopt;
    }

    return read_offer_payload(*mime_type);
  }

  [[nodiscard]] bool write_text(std::string_view text) {
    if (support_ != WaylandClipboardSupport::available || display_ == nullptr ||
        manager_ == nullptr || data_device_ == nullptr) {
      return false;
    }

    {
      std::lock_guard display_lock(display_mutex_);
      auto* source = wl_data_device_manager_create_data_source(manager_);
      if (source == nullptr) {
        return false;
      }

      static const wl_data_source_listener source_listener{
          .target = &Connection::handle_source_target,
          .send = &Connection::handle_source_send,
          .cancelled = &Connection::handle_source_cancelled,
          .dnd_drop_performed = &Connection::handle_source_drop_performed,
          .dnd_finished = &Connection::handle_source_dnd_finished,
          .action = &Connection::handle_source_action,
      };
      wl_data_source_add_listener(source, &source_listener, this);
      wl_data_source_offer(source, "text/plain;charset=utf-8");
      wl_data_source_offer(source, "text/plain");

      {
        std::lock_guard selection_lock(owned_selection_mutex_);
        if (owned_source_ != nullptr) {
          wl_data_source_destroy(owned_source_);
        }
        owned_source_ = source;
        owned_text_ = std::string(text);
      }

      wl_data_device_set_selection(data_device_, source, 0);
      if (wl_display_flush(display_) == -1) {
        std::lock_guard selection_lock(owned_selection_mutex_);
        if (owned_source_ == source) {
          wl_data_source_destroy(owned_source_);
          owned_source_ = nullptr;
          owned_text_.clear();
        }
        return false;
      }
    }

    start_dispatch_thread();
    return true;
  }

 private:
  explicit Connection(std::string_view display_name)
      : display_name_(display_name) {}

  void initialize() {
    display_ =
        wl_display_connect(display_name_.empty() ? nullptr : display_name_.c_str());
    if (display_ == nullptr) {
      support_ = WaylandClipboardSupport::unsupported;
      return;
    }

    registry_ = wl_display_get_registry(display_);
    if (registry_ == nullptr) {
      support_ = WaylandClipboardSupport::unsupported;
      return;
    }

    static const wl_registry_listener registry_listener{
        .global = &Connection::handle_global,
        .global_remove = &Connection::handle_global_remove,
    };
    wl_registry_add_listener(registry_, &registry_listener, this);
    if (wl_display_roundtrip(display_) == -1) {
      support_ = WaylandClipboardSupport::unsupported;
      return;
    }

    if (manager_ == nullptr) {
      support_ = WaylandClipboardSupport::unsupported;
      return;
    }
    if (seat_ == nullptr) {
      support_ = WaylandClipboardSupport::no_seat;
      return;
    }

    data_device_ = wl_data_device_manager_get_data_device(manager_, seat_);
    if (data_device_ == nullptr) {
      support_ = WaylandClipboardSupport::unsupported;
      return;
    }

    static const wl_data_device_listener data_device_listener{
        .data_offer = &Connection::handle_data_offer,
        .enter = &Connection::handle_enter,
        .leave = &Connection::handle_leave,
        .motion = &Connection::handle_motion,
        .drop = &Connection::handle_drop,
        .selection = &Connection::handle_selection,
    };
    wl_data_device_add_listener(data_device_, &data_device_listener, this);
    support_ = WaylandClipboardSupport::available;
    (void)wl_display_roundtrip(display_);
  }

  static void handle_global(
      void* data,
      wl_registry* registry,
      std::uint32_t name,
      const char* interface,
      std::uint32_t version) {
    auto* connection = static_cast<Connection*>(data);
    const std::string_view interface_name(interface);
    if (interface_name == wl_data_device_manager_interface.name) {
      connection->manager_ =
          static_cast<wl_data_device_manager*>(wl_registry_bind(
              registry,
              name,
              &wl_data_device_manager_interface,
              std::min<std::uint32_t>(version, 3)));
      return;
    }
    if (interface_name == wl_seat_interface.name) {
      connection->seat_ = static_cast<wl_seat*>(wl_registry_bind(
          registry,
          name,
          &wl_seat_interface,
          std::min<std::uint32_t>(version, 5)));
    }
  }

  static void handle_global_remove(
      void*,
      wl_registry*,
      std::uint32_t) {}

  static void handle_data_offer(
      void* data,
      wl_data_device*,
      wl_data_offer* offer) {
    auto* connection = static_cast<Connection*>(data);
    connection->clear_offer(connection->pending_offer_);
    connection->pending_offer_ = std::make_unique<Offer>();
    connection->pending_offer_->offer = offer;
    static const wl_data_offer_listener offer_listener{
        .offer = &Connection::handle_offer_mime_type,
        .source_actions = &Connection::handle_offer_source_actions,
        .action = &Connection::handle_offer_action,
    };
    wl_data_offer_add_listener(
        offer,
        &offer_listener,
        connection->pending_offer_.get());
  }

  static void handle_offer_mime_type(
      void* data,
      wl_data_offer*,
      const char* mime_type) {
    auto* offer = static_cast<Offer*>(data);
    if (offer != nullptr && mime_type != nullptr) {
      offer->mime_types.emplace_back(mime_type);
    }
  }

  static void handle_offer_source_actions(
      void*,
      wl_data_offer*,
      std::uint32_t) {}

  static void handle_offer_action(void*, wl_data_offer*, std::uint32_t) {}

  static void handle_enter(
      void*,
      wl_data_device*,
      std::uint32_t,
      wl_surface*,
      wl_fixed_t,
      wl_fixed_t,
      wl_data_offer*) {}

  static void handle_leave(void*, wl_data_device*) {}

  static void handle_motion(
      void*,
      wl_data_device*,
      std::uint32_t,
      wl_fixed_t,
      wl_fixed_t) {}

  static void handle_drop(void*, wl_data_device*) {}

  static void handle_selection(
      void* data,
      wl_data_device*,
      wl_data_offer* offer) {
    auto* connection = static_cast<Connection*>(data);
    if (offer == nullptr) {
      connection->clear_offer(connection->selection_offer_);
      return;
    }

    if (connection->pending_offer_ != nullptr &&
        connection->pending_offer_->offer == offer) {
      connection->clear_offer(connection->selection_offer_);
      connection->selection_offer_ = std::move(connection->pending_offer_);
    }
  }

  void clear_offer(std::unique_ptr<Offer>& offer) {
    if (offer != nullptr && offer->offer != nullptr) {
      wl_data_offer_destroy(offer->offer);
    }
    offer.reset();
  }

  void clear_owned_source() {
    std::lock_guard display_lock(display_mutex_);
    std::lock_guard selection_lock(owned_selection_mutex_);
    if (owned_source_ != nullptr) {
      wl_data_source_destroy(owned_source_);
      owned_source_ = nullptr;
    }
    owned_text_.clear();
  }

  static bool accepts_text_mime_type(const char* mime_type) {
    if (mime_type == nullptr) {
      return false;
    }
    const std::string_view value(mime_type);
    return value == std::string_view{"text/plain;charset=utf-8"} ||
           value == std::string_view{"text/plain"};
  }

  static void handle_source_target(
      void*,
      wl_data_source*,
      const char*) {}

  static void handle_source_send(
      void* data,
      wl_data_source* source,
      const char* mime_type,
      std::int32_t fd) {
    auto* connection = static_cast<Connection*>(data);
    if (connection == nullptr || fd < 0) {
      if (fd >= 0) {
        close(fd);
      }
      return;
    }

    std::string payload;
    {
      std::lock_guard lock(connection->owned_selection_mutex_);
      if (connection->owned_source_ != source ||
          !accepts_text_mime_type(mime_type)) {
        close(fd);
        return;
      }
      payload = connection->owned_text_;
    }

    write_payload_to_fd(payload, fd);
  }

  static void handle_source_cancelled(void* data, wl_data_source* source) {
    auto* connection = static_cast<Connection*>(data);
    if (connection == nullptr || source == nullptr) {
      return;
    }

    bool should_destroy = false;
    {
      std::lock_guard lock(connection->owned_selection_mutex_);
      if (connection->owned_source_ == source) {
        connection->owned_source_ = nullptr;
        connection->owned_text_.clear();
        should_destroy = true;
      }
    }
    if (should_destroy) {
      wl_data_source_destroy(source);
    }
  }

  static void handle_source_drop_performed(void*, wl_data_source*) {}
  static void handle_source_dnd_finished(void*, wl_data_source*) {}
  static void handle_source_action(void*, wl_data_source*, std::uint32_t) {}

  static void write_payload_to_fd(const std::string& payload, int fd) {
    std::size_t written = 0;
    while (written < payload.size()) {
      const auto count = write(
          fd,
          payload.data() + written,
          static_cast<unsigned int>(payload.size() - written));
      if (count == -1 && errno == EINTR) {
        continue;
      }
      if (count <= 0) {
        break;
      }
      written += static_cast<std::size_t>(count);
    }
    close(fd);
  }

  void start_dispatch_thread() {
    bool expected = false;
    if (!dispatch_running_.compare_exchange_strong(expected, true)) {
      return;
    }
    dispatch_thread_ = std::thread([this] { dispatch_owned_selection_events(); });
  }

  void stop_dispatch_thread() {
    dispatch_running_.store(false);
    if (dispatch_thread_.joinable()) {
      dispatch_thread_.join();
    }
  }

  void dispatch_owned_selection_events() {
    if (display_ == nullptr) {
      dispatch_running_.store(false);
      return;
    }

    const int display_fd = wl_display_get_fd(display_);
    while (dispatch_running_.load()) {
      std::lock_guard display_lock(display_mutex_);
      while (wl_display_prepare_read(display_) != 0) {
        if (wl_display_dispatch_pending(display_) == -1) {
          dispatch_running_.store(false);
          return;
        }
      }
      (void)wl_display_flush(display_);

      pollfd descriptor{
          .fd = display_fd,
          .events = POLLIN | POLLHUP | POLLERR,
          .revents = 0,
      };
      const int ready = poll(&descriptor, 1, 50);
      if (ready == -1 && errno == EINTR) {
        wl_display_cancel_read(display_);
        continue;
      }
      if (ready <= 0 || (descriptor.revents & POLLIN) == 0) {
        wl_display_cancel_read(display_);
        continue;
      }
      if (wl_display_read_events(display_) == -1) {
        dispatch_running_.store(false);
        return;
      }
      if (wl_display_dispatch_pending(display_) == -1) {
        dispatch_running_.store(false);
        return;
      }
    }
  }

  [[nodiscard]] std::optional<std::string> preferred_text_mime_type() const {
    if (selection_offer_ == nullptr) {
      return std::nullopt;
    }

    const auto& mime_types = selection_offer_->mime_types;
    const auto utf8 = std::ranges::find(
        mime_types,
        std::string_view{"text/plain;charset=utf-8"});
    if (utf8 != mime_types.end()) {
      return *utf8;
    }

    const auto plain =
        std::ranges::find(mime_types, std::string_view{"text/plain"});
    if (plain != mime_types.end()) {
      return *plain;
    }

    return std::nullopt;
  }

  [[nodiscard]] std::optional<std::string> read_offer_payload(
      const std::string& mime_type) {
    if (selection_offer_ == nullptr || selection_offer_->offer == nullptr) {
      return std::nullopt;
    }

    int pipe_fds[2] = {-1, -1};
    if (pipe2(pipe_fds, O_CLOEXEC) == -1) {
      return std::nullopt;
    }

    wl_data_offer_receive(selection_offer_->offer, mime_type.c_str(), pipe_fds[1]);
    if (wl_display_flush(display_) == -1) {
      close(pipe_fds[0]);
      close(pipe_fds[1]);
      return std::nullopt;
    }

    close(pipe_fds[1]);
    pipe_fds[1] = -1;

    std::string payload;
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (std::chrono::steady_clock::now() < deadline) {
      pollfd descriptor{
          .fd = pipe_fds[0],
          .events = POLLIN | POLLHUP,
          .revents = 0,
      };
      const int ready = poll(&descriptor, 1, 50);
      if (ready == -1 && errno == EINTR) {
        continue;
      }
      if (ready <= 0) {
        continue;
      }

      char buffer[4096];
      const auto bytes_read = read(pipe_fds[0], buffer, sizeof(buffer));
      if (bytes_read > 0) {
        payload.append(buffer, static_cast<std::size_t>(bytes_read));
        continue;
      }
      close(pipe_fds[0]);
      return bytes_read == 0 ? std::optional<std::string>{std::move(payload)}
                             : std::nullopt;
    }

    close(pipe_fds[0]);
    return std::nullopt;
  }

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

std::optional<std::string> MemoryClipboard::read_text() const {
  return text_;
}

bool MemoryClipboard::write_text(std::string_view text) {
  text_ = std::string(text);
  return true;
}

#if defined(__linux__)
WaylandClipboard::WaylandClipboard() = default;

WaylandClipboard::WaylandClipboard(WaylandClipboardOptions options)
    : support_(
          !options.data_device_manager_available
              ? WaylandClipboardSupport::unsupported
              : (options.seat_available ? WaylandClipboardSupport::available
                                        : WaylandClipboardSupport::no_seat)) {
  if (options.connect_to_display) {
    connection_ = Connection::create(options.display_name);
    support_ = connection_ != nullptr ? connection_->support()
                                      : WaylandClipboardSupport::unsupported;
  }
}

WaylandClipboard::~WaylandClipboard() = default;

std::optional<std::string> WaylandClipboard::read_text() const {
  if (connection_ != nullptr) {
    if (auto text = connection_->read_text(); text.has_value()) {
      return text;
    }
    return fallback_.read_text();
  }
  return fallback_.read_text();
}

bool WaylandClipboard::write_text(std::string_view text) {
  const bool fallback_written = fallback_.write_text(text);
  if (connection_ != nullptr && connection_->write_text(text)) {
    return true;
  }
  return fallback_written;
}

WaylandClipboardSupport WaylandClipboard::support() const {
  return support_;
}
#endif

std::unique_ptr<Clipboard> create_platform_clipboard() {
#if defined(_WIN32)
  return std::make_unique<Win32Clipboard>();
#elif defined(__linux__)
  return std::make_unique<WaylandClipboard>();
#else
  return std::make_unique<MemoryClipboard>();
#endif
}

} // namespace cgpui

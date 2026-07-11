#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

struct wl_resource;

namespace cgpui::test {

struct WaylandTestClipboardClientSource {
  void* compositor = nullptr;
  wl_resource* resource = nullptr;
  std::vector<std::string> mime_types;
};

struct WaylandTestClipboardPayloadRequest {
  std::string mime_type;
  bool nonblocking = false;
  bool abandoned = false;
  std::chrono::milliseconds read_delay{0};
};

class WaylandTestClipboardSourceState {
 public:
  void offer(WaylandTestClipboardClientSource* source, const char* mime_type) {
    if (source == nullptr || mime_type == nullptr) return;
    std::lock_guard lock(selection_mutex_);
    source->mime_types.emplace_back(mime_type);
  }

  void set_selection(
      WaylandTestClipboardClientSource* source,
      std::uint32_t serial) {
    std::lock_guard lock(selection_mutex_);
    if (source != nullptr) {
      if (set_count_.load() > 0 && active_source_ == nullptr) {
        replacement_continuous_.store(false);
      }
      active_source_ = source;
      last_serial_.store(serial);
      set_count_.fetch_add(1);
    } else {
      active_source_ = nullptr;
    }
    selection_set_.store(source != nullptr);
    payload_received_.store(false);
  }

  void destroyed(WaylandTestClipboardClientSource* source) {
    std::lock_guard lock(selection_mutex_);
    if (active_source_ == source) {
      active_source_ = nullptr;
      selection_set_.store(false);
    }
  }

  [[nodiscard]] WaylandTestClipboardClientSource* active_source() const {
    std::lock_guard lock(selection_mutex_);
    return active_source_;
  }

  [[nodiscard]] std::vector<std::string> active_mime_types() const {
    std::lock_guard lock(selection_mutex_);
    return active_source_ == nullptr
        ? std::vector<std::string>{}
        : active_source_->mime_types;
  }

  void request_payload(
      std::string_view mime_type,
      bool nonblocking = false,
      bool abandoned = false,
      std::chrono::milliseconds read_delay = std::chrono::milliseconds{0}) {
    {
      std::lock_guard lock(request_mutex_);
      pending_request_ = WaylandTestClipboardPayloadRequest{
          .mime_type = std::string(mime_type),
          .nonblocking = nonblocking,
          .abandoned = abandoned,
          .read_delay = read_delay,
      };
    }
    {
      std::lock_guard lock(payload_mutex_);
      last_payload_.clear();
    }
    payload_received_.store(false);
    request_pending_.store(true);
  }

  [[nodiscard]] std::optional<WaylandTestClipboardPayloadRequest>
  take_payload_request() {
    if (!request_pending_.exchange(false)) return std::nullopt;
    std::lock_guard lock(request_mutex_);
    return pending_request_;
  }

  void retry_payload_request() { request_pending_.store(true); }

  void record_payload(std::string payload) {
    {
      std::lock_guard lock(payload_mutex_);
      last_payload_ = std::move(payload);
    }
    payload_received_.store(true);
  }

  [[nodiscard]] std::string last_payload() const {
    std::lock_guard lock(payload_mutex_);
    return last_payload_;
  }

  [[nodiscard]] bool wait_for_selection_set() const {
    return wait_for([&] { return selection_set_.load(); });
  }
  [[nodiscard]] bool wait_for_set_count(std::uint32_t count) const {
    return wait_for([&] { return set_count_.load() >= count; });
  }
  [[nodiscard]] bool wait_for_payload() const {
    return wait_for([&] { return payload_received_.load(); });
  }
  [[nodiscard]] bool replacement_was_continuous() const {
    return replacement_continuous_.load();
  }

 private:
  template <typename Predicate>
  [[nodiscard]] static bool wait_for(Predicate predicate) {
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!predicate() && std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return predicate();
  }

  mutable std::mutex selection_mutex_;
  WaylandTestClipboardClientSource* active_source_ = nullptr;
  mutable std::mutex request_mutex_;
  WaylandTestClipboardPayloadRequest pending_request_;
  mutable std::mutex payload_mutex_;
  std::string last_payload_;
  std::atomic_bool selection_set_{false};
  std::atomic_bool request_pending_{false};
  std::atomic_bool payload_received_{false};
  std::atomic_bool replacement_continuous_{true};
  std::atomic_uint32_t set_count_{0};
  std::atomic_uint32_t last_serial_{0};
};

} // namespace cgpui::test

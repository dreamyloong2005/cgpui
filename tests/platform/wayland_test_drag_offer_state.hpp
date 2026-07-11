#pragma once

#include <wayland-server-protocol.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

namespace cgpui::test {

class WaylandTestDragOfferState {
 public:
  void install(wl_resource* resource) {
    resource_ = resource;
    accepted_.store(false);
    actions_set_.store(false);
    finished_.store(false);
    destroyed_.store(false);
    std::lock_guard lock(accept_mutex_);
    accepted_mime_.clear();
  }

  void clear_resource() { resource_ = nullptr; }
  [[nodiscard]] bool matches(wl_resource* resource) const {
    return resource_ == resource;
  }

  void record_destroyed(wl_resource* resource) {
    if (!matches(resource)) return;
    resource_ = nullptr;
    destroyed_.store(true);
  }

  void record_accept(const char* mime_type) {
    {
      std::lock_guard lock(accept_mutex_);
      accepted_mime_ = mime_type == nullptr ? std::string{} : std::string(mime_type);
    }
    accepted_.store(true);
  }

  [[nodiscard]] bool record_finish(std::uint32_t selected_action) {
    if (accepted_mime_type().empty() ||
        selected_action == WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE) return false;
    finished_.store(true);
    return true;
  }

  void record_actions(std::uint32_t actions, std::uint32_t preferred) {
    actions_.store(actions);
    preferred_action_.store(preferred);
    actions_set_.store(true);
  }

  [[nodiscard]] bool wait_for_accepted() const { return wait_for(accepted_); }
  [[nodiscard]] bool wait_for_actions() const { return wait_for(actions_set_); }
  [[nodiscard]] bool wait_for_finished() const { return wait_for(finished_); }
  [[nodiscard]] bool finished() const { return finished_.load(); }
  [[nodiscard]] bool wait_for_destroyed() const {
    return wait_for(destroyed_, std::chrono::milliseconds{500});
  }
  [[nodiscard]] std::string accepted_mime_type() const {
    std::lock_guard lock(accept_mutex_);
    return accepted_mime_;
  }
  [[nodiscard]] std::uint32_t actions() const { return actions_.load(); }
  [[nodiscard]] std::uint32_t preferred_action() const {
    return preferred_action_.load();
  }

 private:
  [[nodiscard]] static bool wait_for(
      const std::atomic_bool& flag,
      std::chrono::milliseconds timeout = std::chrono::seconds{3}) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!flag.load() && std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
    return flag.load();
  }

  wl_resource* resource_ = nullptr;
  mutable std::mutex accept_mutex_;
  std::string accepted_mime_;
  std::atomic_bool accepted_{false};
  std::atomic_bool actions_set_{false};
  std::atomic_bool finished_{false};
  std::atomic_bool destroyed_{false};
  std::atomic_uint32_t actions_{WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE};
  std::atomic_uint32_t preferred_action_{WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE};
};

} // namespace cgpui::test

#pragma once

#include "wayland_portal_test_messages.hpp"

#include <atomic>
#include <cstddef>
#include <map>
#include <mutex>
#include <string>
#include <sys/types.h>
#include <thread>
#include <utility>
#include <vector>

namespace cgpui::test {

// A private dbus-daemon with a minimal configuration (no service
// activation), so tests never reach a real desktop portal.
class PrivateSessionBus {
 public:
  PrivateSessionBus() = default;
  ~PrivateSessionBus();
  PrivateSessionBus(const PrivateSessionBus&) = delete;
  PrivateSessionBus& operator=(const PrivateSessionBus&) = delete;

  // False when dbus-daemon is not installed or fails to start.
  [[nodiscard]] bool start();
  [[nodiscard]] const std::string& address() const;

 private:
  std::string address_;
  std::string config_path_;
  pid_t pid_ = -1;
};

// Serves org.freedesktop.portal.Settings ReadOne/Read on a bus from a
// background thread and emits SettingChanged on request.
class FakePortal {
 public:
  FakePortal(std::string address, bool supports_read_one);
  ~FakePortal();
  FakePortal(const FakePortal&) = delete;
  FakePortal& operator=(const FakePortal&) = delete;

  void set_value(std::string name_space, std::string key, PortalValue value);
  [[nodiscard]] bool start();
  void emit(std::string name_space, std::string key, PortalValue value);
  [[nodiscard]] std::size_t read_one_calls() const;
  [[nodiscard]] std::size_t read_calls() const;

 private:
  struct Emission {
    std::string name_space;
    std::string key;
    PortalValue value;
  };

  void run();
  void send_emissions();
  DBusHandlerResult handle(DBusMessage* message);
  static DBusHandlerResult filter(
      DBusConnection* connection,
      DBusMessage* message,
      void* data);

  std::string address_;
  bool supports_read_one_ = true;
  DBusConnection* connection_ = nullptr;
  std::thread thread_;
  std::atomic_bool stop_{false};
  mutable std::mutex mutex_;
  std::map<std::pair<std::string, std::string>, PortalValue> values_;
  std::vector<Emission> emissions_;
  std::atomic<std::size_t> read_one_calls_{0};
  std::atomic<std::size_t> read_calls_{0};
};

} // namespace cgpui::test

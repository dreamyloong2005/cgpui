#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace cgpui::test {

class WaylandTestCompositor {
 public:
  explicit WaylandTestCompositor(std::string name);
  ~WaylandTestCompositor();

  WaylandTestCompositor(const WaylandTestCompositor&) = delete;
  WaylandTestCompositor& operator=(const WaylandTestCompositor&) = delete;

  [[nodiscard]] bool start();
  void stop();

  [[nodiscard]] const std::string& socket_name() const;

  void set_close_on_initial_configure_ack(bool enabled);
  void request_close();
  void request_resize_configure(std::int32_t width, std::int32_t height);

  [[nodiscard]] bool wait_for_close_sent() const;
  [[nodiscard]] bool wait_for_resize_configure_sent() const;
  [[nodiscard]] bool wait_for_resize_configure_acked() const;

 private:
  struct State;
  std::unique_ptr<State> state_;
};

} // namespace cgpui::test

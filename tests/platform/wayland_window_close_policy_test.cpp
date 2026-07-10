#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

bool wait_for(const std::atomic_bool& flag) {
  const auto deadline = std::chrono::steady_clock::now() +
      std::chrono::seconds(3);
  while (!flag.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return flag.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("close-policy");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  cgpui::PlatformWindow* observed_window = nullptr;
  int close_event_count = 0;
  bool pending_state_matched = true;
  std::atomic_bool system_close_accepted{false};
  std::atomic_bool run_finished{false};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Close Policy Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        const auto* close = std::get_if<cgpui::WindowCloseRequested>(&event);
        if (close == nullptr || observed_window == nullptr) {
          return;
        }
        close_event_count += 1;
        const auto state = observed_window->close_request_state();
        pending_state_matched = pending_state_matched && state.pending &&
            !state.accepted && state.source == close->source &&
            state.sequence == close->sequence;
        if (close_event_count == 1) {
          observed_window->request_close();
          (void)observed_window->resolve_close_request(
              cgpui::PlatformWindowCloseResolution::cancel);
        } else {
          (void)observed_window->resolve_close_request(
              cgpui::PlatformWindowCloseResolution::accept);
          system_close_accepted.store(true);
          (*app)->quit();
        }
      });
  if (!window) {
    compositor.stop();
    return 3;
  }
  observed_window = window->get();

  (*window)->request_close();
  const auto cancelled = (*window)->close_request_state();
  if (close_event_count != 1 || cancelled.pending || cancelled.accepted ||
      cancelled.source != cgpui::WindowCloseRequestSource::application ||
      cancelled.sequence != 1 || cancelled.cancelled_count != 1 ||
      cancelled.coalesced_count != 1 || (*window)->state().close_requested) {
    compositor.stop();
    return 4;
  }

  int run_result = -1;
  std::thread client([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });
  compositor.request_close();
  if (!wait_for(system_close_accepted) || !wait_for(run_finished)) {
    (*app)->quit();
    compositor.stop();
    client.join();
    return 5;
  }
  client.join();
  compositor.stop();

  const auto accepted = (*window)->close_request_state();
  if (run_result != 0 || close_event_count != 2 || !pending_state_matched) {
    return 6;
  }
  if (accepted.pending || !accepted.accepted ||
      accepted.source != cgpui::WindowCloseRequestSource::window_manager ||
      accepted.sequence != 2 || accepted.accepted_count != 1 ||
      accepted.cancelled_count != 1 || accepted.coalesced_count != 1 ||
      !(*window)->state().close_requested) {
    return 7;
  }
  return 0;
}

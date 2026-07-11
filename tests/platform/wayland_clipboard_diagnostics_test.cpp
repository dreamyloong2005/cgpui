#include "cgpui/platform/clipboard.hpp"
#include "wayland_test_compositor.hpp"

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <string_view>
#include <thread>

namespace {
template <typename Predicate>
bool wait_for(Predicate predicate) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!predicate() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return predicate();
}
} // namespace

int main() {
  std::signal(SIGPIPE, SIG_DFL);
  cgpui::test::WaylandTestCompositor compositor("clipboard-diagnostics");
  if (!compositor.start()) return 1;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  const int result = [&] {
    cgpui::WaylandClipboard clipboard(cgpui::WaylandClipboardOptions{
        .connect_to_display = true,
    });
    constexpr std::string_view payload = "diagnostic payload";
    if (!clipboard.write_text(payload)) return 2;
    const auto installed = clipboard.diagnostics();
    if (installed.operation != cgpui::WaylandClipboardOperation::write ||
        installed.failure != cgpui::WaylandClipboardFailure::none ||
        installed.bytes_transferred != payload.size()) return 3;

    compositor.request_clipboard_client_selection_abandoned("text/plain");
    if (!wait_for([&] {
          return clipboard.diagnostics().failure ==
              cgpui::WaylandClipboardFailure::receiver_closed;
        })) return 4;
    const auto failed = clipboard.diagnostics();
    if (failed.operation != cgpui::WaylandClipboardOperation::send ||
        failed.revision <= installed.revision) return 5;

    compositor.request_clipboard_client_selection("text/plain;charset=utf-8");
    if (!compositor.wait_for_clipboard_client_selection_payload_received()) {
      return 6;
    }
    if (!wait_for([&] {
          const auto diagnostics = clipboard.diagnostics();
          return diagnostics.operation == cgpui::WaylandClipboardOperation::send &&
              diagnostics.failure == cgpui::WaylandClipboardFailure::none &&
              diagnostics.revision > failed.revision;
        })) return 7;
    const auto recovered = clipboard.diagnostics();
    if (recovered.bytes_transferred != payload.size()) return 8;
    return 0;
  }();
  compositor.stop();
  return result;
}

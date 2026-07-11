#include "cgpui/platform/clipboard.hpp"
#include "wayland_test_compositor.hpp"

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <string>

int main() {
  std::signal(SIGPIPE, SIG_IGN);
  std::string chunked_payload(8U * 4096U, 'c');
  chunked_payload.front() = 'A';
  chunked_payload.back() = 'Z';

  cgpui::test::WaylandTestCompositor compositor(
      "clipboard-incremental-transfer");
  compositor.set_clipboard_selection({
      cgpui::test::WaylandClipboardMimePayload{
          .mime_type = "text/plain;charset=utf-8",
          .payload = chunked_payload,
          .transfer_chunk_size = 4096,
          .transfer_chunk_delay = std::chrono::milliseconds(100),
      },
  });
  if (!compositor.start()) return 1;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  const int result = [&] {
    cgpui::WaylandClipboard clipboard(cgpui::WaylandClipboardOptions{
        .connect_to_display = true,
    });
    if (clipboard.support() != cgpui::WaylandClipboardSupport::available) {
      return 2;
    }
    if (!compositor.wait_for_clipboard_selection_sent()) return 3;
    const auto started = std::chrono::steady_clock::now();
    const auto text = clipboard.read_text();
    const auto elapsed = std::chrono::steady_clock::now() - started;
    if (!text || *text != chunked_payload) return 4;
    if (elapsed < std::chrono::milliseconds(500)) return 5;

    const std::string owned_payload(256U * 1024U, 'o');
    if (!clipboard.write_text(owned_payload)) return 6;
    if (!compositor.wait_for_clipboard_client_selection_set_count(1)) return 7;
    compositor.request_clipboard_client_selection_nonblocking(
        "text/plain", std::chrono::milliseconds(1500));
    if (!compositor.wait_for_clipboard_client_selection_payload_received()) {
      return 8;
    }
    if (compositor.last_clipboard_client_selection_payload() != owned_payload) {
      return 9;
    }
    return 0;
  }();
  compositor.stop();
  return result;
}

#include "cgpui/platform/clipboard.hpp"
#include "wayland_test_compositor.hpp"

#include <csignal>
#include <cstdlib>
#include <string_view>

int main() {
  std::signal(SIGPIPE, SIG_DFL);
  cgpui::test::WaylandTestCompositor compositor("clipboard-failure");
  if (!compositor.start()) return 1;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  const int result = [&] {
    cgpui::WaylandClipboard clipboard(cgpui::WaylandClipboardOptions{
        .connect_to_display = true,
    });
    if (clipboard.support() != cgpui::WaylandClipboardSupport::available) {
      return 2;
    }
    constexpr std::string_view payload = "survives abandoned receiver";
    if (!clipboard.write_text(payload)) return 3;
    if (!compositor.wait_for_clipboard_client_selection_set_count(1)) return 4;

    compositor.request_clipboard_client_selection_abandoned("text/plain");
    if (!compositor.wait_for_clipboard_client_selection_abandoned()) return 5;

    compositor.request_clipboard_client_selection("text/plain;charset=utf-8");
    if (!compositor.wait_for_clipboard_client_selection_payload_received()) {
      return 6;
    }
    if (compositor.last_clipboard_client_selection_payload() != payload) {
      return 7;
    }
    return 0;
  }();
  compositor.stop();
  return result;
}

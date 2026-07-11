#include "cgpui/platform/clipboard.hpp"
#include "wayland_test_compositor.hpp"

#include <cstdlib>
#include <string_view>

int main() {
  cgpui::test::WaylandTestCompositor compositor(
      "clipboard-ownership-replacement");
  if (!compositor.start()) return 1;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  const int result = [&] {
    cgpui::WaylandClipboard clipboard(cgpui::WaylandClipboardOptions{
        .connect_to_display = true,
    });
    if (clipboard.support() != cgpui::WaylandClipboardSupport::available) {
      return 2;
    }
    if (!clipboard.write_text("first payload")) return 3;
    if (!compositor.wait_for_clipboard_client_selection_set_count(1)) return 4;
    compositor.request_clipboard_client_selection(
        "text/plain;charset=utf-8");
    if (!compositor.wait_for_clipboard_client_selection_payload_received() ||
        compositor.last_clipboard_client_selection_payload() !=
            std::string_view{"first payload"}) {
      return 5;
    }

    if (!clipboard.write_text("second payload")) return 6;
    if (!compositor.wait_for_clipboard_client_selection_set_count(2) ||
        !compositor.clipboard_client_selection_replacement_was_continuous()) {
      return 7;
    }
    compositor.request_clipboard_client_selection("text/plain");
    if (!compositor.wait_for_clipboard_client_selection_payload_received() ||
        compositor.last_clipboard_client_selection_payload() !=
            std::string_view{"second payload"}) {
      return 8;
    }
    return 0;
  }();
  compositor.stop();
  return result;
}

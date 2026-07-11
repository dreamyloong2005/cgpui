#include "cgpui/platform/clipboard.hpp"
#include "wayland_test_compositor.hpp"

#include <cstdlib>
#include <string_view>

int main() {
  cgpui::test::WaylandTestCompositor compositor(
      "clipboard-read-owned-lifecycle");
  if (!compositor.start()) return 1;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  const int result = [&] {
    cgpui::WaylandClipboard clipboard(cgpui::WaylandClipboardOptions{
        .connect_to_display = true,
    });
    if (clipboard.support() != cgpui::WaylandClipboardSupport::available) {
      return 2;
    }
    if (!clipboard.write_text("owned before read")) return 3;
    if (!compositor.wait_for_clipboard_client_selection_set_count(1)) return 4;

    compositor.set_clipboard_selection({
        cgpui::test::WaylandClipboardMimePayload{
            .mime_type = "text/plain;charset=utf-8",
            .payload = "external replacement \xE4\xB8\xAD",
        },
    });
    if (!compositor.wait_for_clipboard_selection_sent()) return 5;

    const auto text = clipboard.read_text();
    if (!text || *text !=
        std::string_view{"external replacement \xE4\xB8\xAD"}) {
      return 6;
    }
    return 0;
  }();
  compositor.stop();
  return result;
}

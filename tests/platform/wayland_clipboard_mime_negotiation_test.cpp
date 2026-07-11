#include "cgpui/platform/clipboard.hpp"
#include "wayland_test_compositor.hpp"

#include <cstdlib>
#include <string_view>

int main() {
  cgpui::test::WaylandTestCompositor compositor(
      "clipboard-mime-negotiation");
  compositor.set_clipboard_selection({
      cgpui::test::WaylandClipboardMimePayload{
          .mime_type = "text/plain",
          .payload = "plain fallback",
      },
      cgpui::test::WaylandClipboardMimePayload{
          .mime_type = "Text/Plain; Charset=UTF-8",
          .payload = "normalized utf8 \xE4\xB8\xAD",
      },
      cgpui::test::WaylandClipboardMimePayload{
          .mime_type = "image/png",
          .payload = "not text",
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
    const auto text = clipboard.read_text();
    if (!text || *text != std::string_view{"normalized utf8 \xE4\xB8\xAD"}) {
      return 4;
    }
    if (compositor.last_clipboard_receive_mime_type() !=
        std::string_view{"Text/Plain; Charset=UTF-8"}) {
      return 5;
    }

    compositor.set_clipboard_selection({
        cgpui::test::WaylandClipboardMimePayload{
            .mime_type = "text/plain;charset=iso-8859-1",
            .payload = "unsupported charset",
        },
        cgpui::test::WaylandClipboardMimePayload{
            .mime_type = "TEXT/PLAIN",
            .payload = "case folded plain",
        },
    });
    if (!compositor.wait_for_clipboard_selection_sent()) return 6;
    const auto fallback = clipboard.read_text();
    if (!fallback || *fallback != std::string_view{"case folded plain"}) {
      return 7;
    }
    if (compositor.last_clipboard_receive_mime_type() !=
        std::string_view{"TEXT/PLAIN"}) {
      return 8;
    }
    return 0;
  }();
  compositor.stop();
  return result;
}

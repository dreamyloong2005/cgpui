#include "cgpui/platform/clipboard.hpp"
#include "wayland_test_compositor.hpp"

#include <cstdlib>
#include <string>
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
    const auto baseline = clipboard.diagnostics();
    if (baseline.owns_selection || baseline.owned_payload_bytes != 0 ||
        baseline.ownership_revision != 0) return 3;
    std::string last_payload;
    for (std::uint64_t index = 0; index < 64; ++index) {
      last_payload = "ownership payload " + std::to_string(index);
      if (!clipboard.write_text(last_payload)) return 4;
      if (!compositor.wait_for_clipboard_client_selection_set_count(
              static_cast<std::uint32_t>(index + 1))) return 5;
      const auto diagnostics = clipboard.diagnostics();
      if (!diagnostics.owns_selection ||
          diagnostics.owned_payload_bytes != last_payload.size() ||
          diagnostics.ownership_revision != index + 1) return 6;
    }
    if (!compositor.clipboard_client_selection_replacement_was_continuous())
      return 7;
    compositor.request_clipboard_client_selection("text/plain");
    if (!compositor.wait_for_clipboard_client_selection_payload_received() ||
        compositor.last_clipboard_client_selection_payload() !=
            last_payload) {
      return 8;
    }
    return 0;
  }();
  compositor.stop();
  return result;
}

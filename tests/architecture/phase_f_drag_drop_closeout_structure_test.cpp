#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines;
}
} // namespace

int main() {
  const std::string win32_source =
      read_source("src/platform/win32/win32_ole_drop_source.cpp");
  const std::string win32_behavior =
      read_source("tests/platform/win32_ole_drop_source_test.cpp");
  const std::string wayland_events =
      read_source("src/platform/linux/wayland_data_device_drag_events.cpp");
  const std::string wayland_offer =
      read_source("src/platform/linux/wayland_data_device_offer.cpp");
  const std::string wayland_behavior =
      read_source("tests/platform/wayland_drag_cancel_test.cpp");
  const std::string offer_state =
      read_source("tests/platform/wayland_test_drag_offer_state.hpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &win32_source, &win32_behavior, &wayland_events, &wayland_offer,
      &wayland_behavior, &offer_state, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(win32_source, "result == DRAGDROP_S_DROP ? effect") ||
      !contains(win32_source, ": DROPEFFECT_NONE") ||
      !contains(win32_behavior, "cancel_drag(") ||
      !contains(win32_behavior, "cancelled.effect != DROPEFFECT_NONE")) return 2;
  if (!contains(wayland_events, "WaylandDataDevice::handle_leave(") ||
      !contains(wayland_events, "self->drag_window_ = nullptr") ||
      !contains(wayland_events, "self->clear_active_offer()") ||
      !contains(wayland_offer, "wl_data_offer_destroy(offer->offer)")) return 3;
  if (!contains(wayland_behavior, "request_drag_leave()") ||
      !contains(wayland_behavior, "request_drag_drop()") ||
      !contains(wayland_behavior, "exited.load() != 1") ||
      !contains(wayland_behavior, "dropped.load() != 0") ||
      !contains(wayland_behavior, "drag_offer_finished()")) return 4;
  if (!contains(offer_state, "bool finished() const")) return 5;
  if (line_count(win32_source) > 70 || line_count(win32_behavior) > 75 ||
      line_count(wayland_events) > 95 || line_count(wayland_offer) > 125 ||
      line_count(wayland_behavior) > 120 || line_count(offer_state) > 110) return 6;
  if (!contains(manifests, "\"wayland_drag_cancel_test.cpp\"") ||
      !contains(manifests, "\"phase_f_drag_drop_closeout_structure_test.cpp\"")) return 7;
  return 0;
}

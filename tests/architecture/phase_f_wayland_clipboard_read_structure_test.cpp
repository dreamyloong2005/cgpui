#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
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
  const std::string internal =
      read_source("src/platform/clipboard_wayland_internal.hpp");
  const std::string read =
      read_source("src/platform/clipboard_wayland_read.cpp");
  const std::string payload =
      read_source("src/platform/clipboard_wayland_read_payload.cpp");
  const std::string behavior =
      read_source("tests/platform/wayland_clipboard_read_lifecycle_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &internal, &read, &payload, &behavior, &manifests};
  for (const std::string* value : required) if (value->empty()) return 1;
  if (!contains(internal, "std::optional<std::string> read_text()") ||
      !contains(internal, "owned_selection_mutex_")) return 2;
  const auto stop = read.find("stop_dispatch_thread();");
  const auto lock = read.find("std::lock_guard display_lock", stop);
  const auto roundtrip = read.find("wl_display_roundtrip(", lock);
  const auto mime = read.find("preferred_text_mime_type()", roundtrip);
  const auto receive = read.find("read_offer_payload(*mime_type)", mime);
  const auto ownership = read.find("owned_source_ != nullptr", receive);
  const auto restart = read.find("start_dispatch_thread()", ownership);
  if (stop == std::string::npos || lock < stop || roundtrip < lock ||
      mime < roundtrip || receive < mime || ownership < receive ||
      restart < ownership) return 3;
  if (!contains(payload, "wl_data_offer_receive") ||
      !contains(payload, "std::chrono::seconds(3)") ||
      !contains(payload, "std::optional<std::string>{std::move(payload)}")) {
    return 4;
  }
  if (!contains(behavior, "owned before read") ||
      !contains(behavior, "external replacement") ||
      !contains(behavior, "wait_for_clipboard_selection_sent")) return 5;
  if (!contains(manifests,
                "\"wayland_clipboard_read_lifecycle_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_wayland_clipboard_read_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(internal) > 150 || line_count(read) > 70 ||
      line_count(payload) > 80 || line_count(behavior) > 80) return 7;
  return 0;
}

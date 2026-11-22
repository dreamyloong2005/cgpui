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
  const std::string transfer =
      read_source("src/platform/clipboard_wayland_transfer.cpp");
  const std::string source_io =
      read_source("src/platform/clipboard_wayland_source_io.cpp");
  const std::string read_payload =
      read_source("src/platform/clipboard_wayland_read_payload.cpp");
  const std::string helper =
      read_source("tests/platform/wayland_test_clipboard_transfer.hpp");
  const std::string source_state =
      read_source("tests/platform/wayland_test_clipboard_source_state.hpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string compositor_header =
      read_source("tests/platform/wayland_test_compositor.hpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_clipboard_incremental_transfer_test.cpp");
  const std::string platform_structure =
      read_source("tests/architecture/platform_source_structure_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &internal, &transfer, &source_io, &read_payload, &helper, &source_state,
      &compositor, &compositor_header, &behavior, &platform_structure, &manifests};
  for (const std::string* value : required) if (value->empty()) return 1;
  if (!contains(internal, "wayland_clipboard_write_payload_incrementally(") ||
      !contains(transfer, "EAGAIN") || !contains(transfer, "EWOULDBLOCK") ||
      !contains(transfer, "EINTR") || !contains(transfer, "POLLOUT") ||
      !contains(transfer, "deadline = std::chrono::steady_clock::now()")) {
    return 2;
  }
  if (!contains(source_io,
                "wayland_clipboard_write_payload_incrementally(fd, payload)") ||
      !contains(read_payload,
                "deadline = std::chrono::steady_clock::now()") ||
      !contains(read_payload, "bytes_read == -1 && errno == EINTR")) return 3;
  if (!contains(helper, "fill_wayland_test_clipboard_pipe(") ||
      !contains(helper, "write_wayland_test_clipboard_payload(") ||
      !contains(source_state, "WaylandTestClipboardPayloadRequest") ||
      !contains(compositor, "O_NONBLOCK") ||
      !contains(compositor, "payload.erase(0, preloaded)") ||
      !contains(compositor_header,
                "request_clipboard_client_selection_nonblocking(")) return 4;
  if (!contains(behavior, "transfer_chunk_size = 4096") ||
      !contains(behavior, "256U * 1024U") ||
      !contains(behavior,
                "request_clipboard_client_selection_nonblocking(")) return 5;
  if (!contains(platform_structure,
                "\"src/platform/clipboard_wayland_transfer.cpp\"") ||
      !contains(manifests,
                "\"wayland_clipboard_incremental_transfer_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_wayland_clipboard_incremental_transfer_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(internal) > 150 || line_count(transfer) > 80 ||
      line_count(source_io) > 30 || line_count(read_payload) > 80 ||
      line_count(helper) > 70 || line_count(source_state) > 160 ||
      line_count(compositor) > 3700 || line_count(compositor_header) > 220 ||
      line_count(behavior) > 90) return 7;
  return 0;
}

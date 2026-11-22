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
  const std::string signal =
      read_source("src/platform/clipboard_wayland_signal.cpp");
  const std::string transfer =
      read_source("src/platform/clipboard_wayland_transfer.cpp");
  const std::string source_state =
      read_source("tests/platform/wayland_test_clipboard_source_state.hpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string compositor_header =
      read_source("tests/platform/wayland_test_compositor.hpp");
  const std::string behavior =
      read_source("tests/platform/wayland_clipboard_failure_test.cpp");
  const std::string platform_structure =
      read_source("tests/architecture/platform_source_structure_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &internal, &signal, &transfer, &source_state, &compositor,
      &compositor_header, &behavior, &platform_structure, &manifests};
  for (const std::string* value : required) if (value->empty()) return 1;
  if (!contains(internal, "wayland_clipboard_write_without_sigpipe(") ||
      !contains(signal, "pthread_sigmask(SIG_BLOCK") ||
      !contains(signal, "sigpending(") || !contains(signal, "sigtimedwait(") ||
      !contains(signal, "pthread_sigmask(SIG_SETMASK") ||
      !contains(signal, "write_error == EPIPE")) return 2;
  if (!contains(transfer, "wayland_clipboard_write_without_sigpipe(")) return 3;
  if (!contains(source_state, "bool abandoned = false") ||
      !contains(compositor, "mime_type->abandoned") ||
      !contains(compositor,
                "clipboard_abandoned_request_sent.store(true)") ||
      !contains(compositor_header,
                "request_clipboard_client_selection_abandoned(")) return 4;
  if (!contains(behavior, "std::signal(SIGPIPE, SIG_DFL)") ||
      !contains(behavior,
                "request_clipboard_client_selection_abandoned(") ||
      !contains(behavior,
                "request_clipboard_client_selection(\"text/plain;charset=utf-8\")") ||
      !contains(behavior, "survives abandoned receiver")) return 5;
  if (!contains(platform_structure,
                "\"src/platform/clipboard_wayland_signal.cpp\"") ||
      !contains(manifests, "\"wayland_clipboard_failure_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_wayland_clipboard_failure_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(internal) > 150 || line_count(signal) > 80 ||
      line_count(transfer) > 80 || line_count(source_state) > 160 ||
      line_count(compositor) > 3700 || line_count(compositor_header) > 220 ||
      line_count(behavior) > 70) return 7;
  return 0;
}

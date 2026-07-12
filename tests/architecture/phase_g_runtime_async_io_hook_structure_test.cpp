#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
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
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string public_header =
      read_source("include/cgpui/ui/async_io_hook.hpp");
  const std::string runtime_header =
      read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string context_header =
      read_source("include/cgpui/ui/runtime_context.hpp");
  const std::string async_header =
      read_source("include/cgpui/ui/async_context.hpp");
  const std::string internal =
      read_source("src/ui/runtime_async_io_internal.hpp");
  const std::string hook_source = read_source("src/ui/async_io_hook.cpp");
  const std::string registry_source =
      read_source("src/ui/runtime_async_io.cpp");
  const std::string completion_source =
      read_source("src/ui/runtime_async_io_completion.cpp");
  const std::string scheduling = read_source("src/ui/runtime_scheduling.cpp");
  const std::string shutdown = read_source("src/ui/runtime_shutdown.cpp");
  const std::string behavior =
      read_source("tests/async/runtime_async_io_hook_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_runtime_task_cancellation_propagation_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core_parity = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &public_header, &runtime_header, &context_header, &async_header,
      &internal, &hook_source, &registry_source, &completion_source,
      &scheduling, &shutdown, &behavior, &ui_structure, &header_cleanliness,
      &previous, &xmake, &vocabulary, &core_parity, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(public_header, "using AsyncIoResult = Result<AsyncIoPayload>") ||
      !contains(public_header, "class AsyncIoHook") ||
      !contains(public_header, "bool notify(AsyncIoPayload payload) const") ||
      !contains(public_header, "bool notify_error(Error error) const") ||
      !contains(public_header, "bool cancel() const")) return 2;
  if (!contains(runtime_header, "create_async_io_hook(") ||
      !contains(context_header, "create_async_io_hook(") ||
      !contains(async_header, "create_async_io_hook(")) return 3;
  if (!contains(internal, "class WindowRuntime::RuntimeAsyncIoRegistry") ||
      !contains(internal, "std::mutex mutex_") ||
      !contains(internal, "std::vector<Completion> completions_") ||
      !contains(internal, "AsyncIoHookStatus")) return 4;
  if (!contains(registry_source, "RuntimeAsyncIoRegistry::notify(") ||
      !contains(registry_source, "AsyncIoHookStatus::queued") ||
      !contains(registry_source, "std::erase_if(") ||
      !contains(registry_source, "request_platform_wakeup()")) return 5;
  if (!contains(completion_source,
                "runtime_task_priorities_descending") ||
      !contains(completion_source, "callback(context(), completion.result)") ||
      !contains(scheduling, "drain_async_io_completions()") ||
      !contains(shutdown, "async_io_registry_->shutdown()")) return 6;
  if (!contains(behavior, "std::thread low_thread") ||
      !contains(behavior, "duplicate_suppressed") ||
      !contains(behavior, "std::vector<int>({3, 2, 1})") ||
      !contains(behavior, "callbacks_on_runtime_thread") ||
      !contains(behavior, "detached.cancelled()")) return 7;
  if (!contains(ui_structure, "src/ui/runtime_async_io_internal.hpp") ||
      !contains(ui_structure, "src/ui/runtime_async_io_completion.cpp") ||
      !contains(header_cleanliness, "cgpui::AsyncIoHook async_io_hook") ||
      !contains(previous, "639 async I/O hook production behavior is next.") ||
      !contains(xmake, "target(\"phase_g_runtime_async_io_hook_test\")") ||
      !contains(xmake,
                "target(\"phase_g_runtime_async_io_hook_structure_test\")")) {
    return 8;
  }
  if (line_count(public_header) > 70 || line_count(internal) > 80 ||
      line_count(hook_source) > 60 || line_count(registry_source) > 140 ||
      line_count(completion_source) > 40 || line_count(behavior) > 190 ||
      line_count(runtime_header) > 260) return 9;

  constexpr const char* completion =
      "Phase G Step 639 adds one-shot async I/O hooks with thread-safe "
      "success/failure notification, priority-aware runtime-thread dispatch, "
      "explicit cancellation, duplicate suppression, and shutdown detachment. "
      "Step 640 async timer integration production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (!contains(vocabulary, "`AsyncIoHook`") ||
      contains(core_parity, "Async I/O integration and cross-thread") ||
      !contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 644 element lifecycle "
          "animation production behavior\"")) {
    return 11;
  }
  return 0;
}

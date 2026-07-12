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
      read_source("include/cgpui/ui/cross_thread_entity.hpp");
  const std::string async_header =
      read_source("include/cgpui/ui/async_context.hpp");
  const std::string templates =
      read_source("include/cgpui/ui/runtime_templates.hpp");
  const std::string runtime_header =
      read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string runtime_internal =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string queue_internal =
      read_source("src/ui/runtime_cross_thread_entity_internal.hpp");
  const std::string queue_source =
      read_source("src/ui/runtime_cross_thread_entity_queue.cpp");
  const std::string drain_source =
      read_source("src/ui/runtime_cross_thread_entity.cpp");
  const std::string core_source = read_source("src/ui/runtime_core.cpp");
  const std::string scheduling = read_source("src/ui/runtime_scheduling.cpp");
  const std::string shutdown = read_source("src/ui/runtime_shutdown.cpp");
  const std::string behavior =
      read_source("tests/async/runtime_cross_thread_entity_access_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_runtime_async_timer_integration_structure_test.cpp");
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
      &public_header, &async_header, &templates, &runtime_header,
      &runtime_internal, &queue_internal, &queue_source, &drain_source,
      &core_source, &scheduling, &shutdown, &behavior, &ui_structure,
      &header_cleanliness, &previous, &xmake, &vocabulary, &core_parity,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(public_header, "class CrossThreadEntity") ||
      !contains(public_header, "enum class CrossThreadEntityAccessStatus") ||
      !contains(public_header, "ReadCallback") ||
      !contains(public_header, "UpdateCallback") ||
      !contains(public_header, "bool read(") ||
      !contains(public_header, "bool update(") ||
      !contains(public_header, "std::weak_ptr<detail::CrossThreadEntityQueueState>")) {
    return 2;
  }
  if (!contains(async_header, "CrossThreadEntity<T> entity(") ||
      !contains(templates, "AsyncContextCapability::entity(") ||
      !contains(templates, "CrossThreadEntity<T>::read(") ||
      !contains(templates, "CrossThreadEntity<T>::update(") ||
      !contains(templates, "CrossThreadEntityAccessStatus::context_mismatch") ||
      !contains(templates, "CrossThreadEntityAccessStatus::missing")) return 3;
  if (!contains(queue_internal, "class CrossThreadEntityQueueState") ||
      !contains(queue_internal, "std::mutex mutex_") ||
      !contains(queue_internal, "WindowRuntime* runtime_") ||
      !contains(queue_internal, "operations_") ||
      !contains(runtime_internal, "cross_thread_entity_queue_") ||
      !contains(core_source, "make_shared<detail::CrossThreadEntityQueueState>")) {
    return 4;
  }
  if (!contains(queue_source, "std::lock_guard lock(mutex_)") ||
      !contains(queue_source, "operations_.push_back") ||
      !contains(queue_source, "runtime_->request_platform_wakeup()") ||
      !contains(queue_source, "runtime_ = nullptr") ||
      !contains(queue_source, "queue.lock()")) return 5;
  if (!contains(drain_source, "drain_cross_thread_entity_operations()") ||
      !contains(drain_source, "take_operations()") ||
      !contains(drain_source, "operation(context())") ||
      !contains(scheduling, "drain_cross_thread_entity_operations()") ||
      !contains(shutdown, "cross_thread_entity_queue_->shutdown()")) return 6;
  if (!contains(behavior, "std::array<std::atomic_bool, 16>") ||
      !contains(behavior, "std::thread worker") ||
      !contains(behavior, "callbacks_on_runtime_thread") ||
      !contains(behavior, "CrossThreadEntityAccessStatus::missing") ||
      !contains(behavior, "foreign_access.empty()") ||
      !contains(behavior, "!detached_access.read(")) return 7;
  if (!contains(ui_structure, "include/cgpui/ui/cross_thread_entity.hpp") ||
      !contains(ui_structure, "runtime_cross_thread_entity_queue.cpp") ||
      !contains(header_cleanliness, "CrossThreadEntity<TestModel>") ||
      !contains(previous,
                "Step 641 cross-thread entity \"") ||
      !contains(xmake,
                "target(\"phase_g_runtime_cross_thread_entity_access_test\")") ||
      !contains(xmake,
                "target(\"phase_g_runtime_cross_thread_entity_access_structure_test\")")) {
    return 8;
  }
  if (line_count(public_header) > 90 || line_count(async_header) > 120 ||
      line_count(runtime_header) > 260 || line_count(runtime_internal) > 260 ||
      line_count(templates) > 1050 || line_count(queue_internal) > 40 ||
      line_count(queue_source) > 60 || line_count(drain_source) > 30 ||
      line_count(scheduling) > 220 || line_count(shutdown) > 70 ||
      line_count(behavior) > 220) return 9;

  constexpr const char* completion =
      "Phase G Step 641 adds explicit CrossThreadEntity<T> read and update "
      "queueing through AsyncContextCapability, executing FIFO worker-thread "
      "submissions on the owning runtime thread with context isolation, "
      "missing-entity status, concurrent safety, and shutdown detachment. "
      "Step 642 async runtime production closeout audit is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (!contains(vocabulary, "`CrossThreadEntity<T>`") ||
      contains(core_parity, "cross-thread entity access remains incomplete") ||
      contains(core_parity, "Cross-thread entity access guarantees") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 655 asset cache key production behavior\"")) {
    return 11;
  }
  return 0;
}

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
  const std::string header =
      read_source("include/cgpui/ui/async_asset_loading.hpp");
  const std::string source = read_source("src/ui/async_asset_loading.cpp");
  const std::string behavior =
      read_source("tests/assets/async_asset_loading_test.cpp");
  const std::string outcomes =
      read_source("tests/assets/async_asset_loading_outcome_test.cpp");
  const std::string cleanliness = read_source(
      "tests/header_cleanliness/async_asset_loading_header_cleanliness.cpp");
  const std::string prelude = read_source("include/cgpui/prelude.hpp");
  const std::string prelude_cleanliness =
      read_source("tests/header_cleanliness/prelude_header_cleanliness.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_asset_reload_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &header, &source, &behavior, &outcomes, &cleanliness, &prelude,
      &prelude_cleanliness, &previous, &xmake, &vocabulary, &core, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "struct AsyncAssetLoadRequest") ||
      !contains(header, "std::shared_ptr<const AssetSource> source") ||
      !contains(header, "struct AsyncAssetLoadResult") ||
      !contains(header, "using AsyncAssetLoadCallback") ||
      !contains(header, "Result<TaskHandle> load_asset_async(") ||
      contains(header, "<thread>") || contains(header, "<mutex>")) return 2;
  if (!contains(source, "make_asset_cache_key(") ||
      !contains(source, "context.try_spawn_background_task(") ||
      !contains(source, "request.priority") ||
      !contains(source, "context.valid()") ||
      !contains(source, "source.load(path)") ||
      !contains(source, "load_asset_bytes(*source, path)") ||
      !contains(source, "catch (const std::exception& error)") ||
      !contains(source, "async asset load source is required") ||
      !contains(source, "async asset load completion callback is required") ||
      contains(source, "std::thread") || contains(source, "std::jthread") ||
      contains(source, "std::async")) return 3;
  if (!contains(behavior, "callback_on_runtime_thread") ||
      !contains(behavior, "task_pool_completed_work_count") ||
      !contains(behavior, "invalid_key_rejected") ||
      !contains(behavior, "rejects_empty_source_before_context_access") ||
      !contains(outcomes, "missing_matches") ||
      !contains(outcomes, "error_matches") ||
      !contains(outcomes, "cancelled_completion_ran") ||
      !contains(outcomes, "source_released_after_work")) return 4;
  if (!contains(cleanliness, "AsyncAssetLoadRequest request") ||
      !contains(cleanliness, "AsyncAssetLoadResult result") ||
      !contains(previous, "Step 657 async asset ") ||
      !contains(previous, "loading production behavior is next.") ||
      !contains(xmake, "target(\"phase_g_async_asset_loading_test\")") ||
      !contains(xmake,
                "target(\"phase_g_async_asset_loading_outcome_test\")") ||
      !contains(xmake,
                "target(\"async_asset_loading_header_cleanliness\")") ||
      !contains(xmake,
                "target(\"phase_g_async_asset_loading_structure_test\")")) {
    return 5;
  }
  if (!contains(prelude, "cgpui/ui/async_asset_loading.hpp") ||
      !contains(prelude_cleanliness, "prelude_async_asset_request") ||
      !contains(prelude_cleanliness, "prelude_async_asset_callback")) return 6;
  if (line_count(header) > 45 || line_count(source) > 85 ||
      line_count(behavior) > 200 || line_count(outcomes) > 190) return 7;

  constexpr const char* completion =
      "Phase G Step 657 adds bounded priority-aware async asset loading with "
      "owned source lifetimes, normalized cache identities, worker-thread "
      "reads, runtime-thread success, missing, and error completion, "
      "cancellation suppression, and submission diagnostics. Step 658 "
      "official image/GIF examples and asset closeout is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(vocabulary, "`AsyncAssetLoadRequest`") ||
      !contains(vocabulary, "`load_asset_async(...)`") ||
      !contains(core, "Bounded async asset loading") ||
      !contains(ledger_json, "\"phase_g_step_657_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase J Step 799 re-run upstream extractor against the pinned revision\"")) return 9;
  return 0;
}

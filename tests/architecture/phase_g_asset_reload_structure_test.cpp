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
  const std::string header = read_source("include/cgpui/core/asset_reload.hpp");
  const std::string key_header =
      read_source("include/cgpui/core/asset_cache_key.hpp");
  const std::string error = read_source("include/cgpui/core/error.hpp");
  const std::string source = read_source("src/core/asset_reload.cpp");
  const std::string behavior = read_source("tests/assets/asset_reload_test.cpp");
  const std::string core_cleanliness =
      read_source("tests/header_cleanliness/core_header_cleanliness.cpp");
  const std::string prelude = read_source("include/cgpui/prelude.hpp");
  const std::string prelude_cleanliness =
      read_source("tests/header_cleanliness/prelude_header_cleanliness.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_asset_cache_key_structure_test.cpp");
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
      &header, &key_header, &error, &source, &behavior, &core_cleanliness,
      &prelude, &prelude_cleanliness, &previous, &xmake, &vocabulary, &core,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "struct AssetReloadInvalidation") ||
      !contains(header, "struct AssetSourceInvalidation") ||
      !contains(header, "struct AssetReloadSnapshot") ||
      !contains(header, "class AssetReloadState") ||
      !contains(header, "class Impl;") || contains(header, "<mutex>") ||
      contains(header, "<unordered_map>")) return 2;
  if (!contains(source, "std::unordered_map<AssetCacheKey") ||
      !contains(source, "std::scoped_lock lock(impl_->mutex)") ||
      !contains(source, "try_emplace(*identity, 0)") ||
      !contains(source, "std::numeric_limits<std::uint64_t>::max()") ||
      !contains(source, "impl_->invalidation_count += invalidated") ||
      !contains(error, "asset_revision_exhausted")) return 3;
  const std::size_t saturation =
      source.find("revision == std::numeric_limits<std::uint64_t>::max()",
                  source.find("invalidate_source("));
  const std::size_t source_increment =
      source.find("revision += 1", source.find("invalidate_source("));
  if (saturation == std::string::npos || source_increment == std::string::npos ||
      saturation > source_increment) return 4;
  if (!contains(behavior, "test_invalidates_all_variants_for_one_asset") ||
      !contains(behavior, "test_invalidates_a_tracked_source_in_one_operation") ||
      !contains(behavior, "test_serializes_concurrent_invalidations") ||
      !contains(behavior, "invalidations_per_thread = 100") ||
      !contains(core_cleanliness, "header_reloads.snapshot()") ||
      !contains(prelude_cleanliness, "prelude_reloads.snapshot()")) return 5;
  if (!contains(key_header, "std::uint64_t revision") ||
      !contains(prelude, "cgpui/core/asset_reload.hpp") ||
      !contains(previous, "Step 656 asset reload invalidation production ") ||
      !contains(previous, "behavior is next.") ||
      !contains(xmake, "target(\"phase_g_asset_reload_test\")") ||
      !contains(xmake,
                "target(\"phase_g_asset_reload_structure_test\")")) {
    return 6;
  }
  if (line_count(header) > 65 || line_count(source) > 130 ||
      line_count(behavior) > 130) return 7;

  constexpr const char* completion =
      "Phase G Step 656 adds thread-safe asset reload invalidation with shared "
      "source/path revisions across decoded variants, atomic single-asset and "
      "source-wide updates, saturation-safe fail-closed behavior, and "
      "observable tracking and invalidation diagnostics. Step 657 async asset "
      "loading production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(vocabulary, "`AssetReloadState`") ||
      !contains(vocabulary, "`AssetReloadSnapshot`") ||
      !contains(core, "Thread-safe asset reload invalidation") ||
      !contains(ledger_json, "\"phase_g_step_656_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase I Step 759 X11/XCB platform boundary\"")) return 9;
  return 0;
}

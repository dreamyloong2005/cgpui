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
  const std::string header = read_source("include/cgpui/core/asset_source.hpp");
  const std::string error = read_source("include/cgpui/core/error.hpp");
  const std::string internal =
      read_source("src/core/file_asset_source_internal.hpp");
  const std::string path = read_source("src/core/file_asset_path.cpp");
  const std::string source = read_source("src/core/file_asset_source.cpp");
  const std::string prelude = read_source("include/cgpui/prelude.hpp");
  const std::string cleanliness =
      read_source("tests/header_cleanliness/core_header_cleanliness.cpp");
  const std::string behavior =
      read_source("tests/assets/file_asset_source_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_animation_examples_structure_test.cpp");
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
      &header, &error, &internal, &path, &source, &prelude, &cleanliness,
      &behavior, &previous, &xmake, &vocabulary, &core, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "struct AssetBytes") ||
      !contains(header, "class AssetSource") ||
      !contains(header, "class FileAssetSource final") ||
      !contains(header, "FileAssetSourceOptions") ||
      !contains(header, "Result<std::optional<AssetBytes>> load(") ||
      !contains(header, "Result<std::vector<std::string>> list(") ||
      contains(header, "<fstream>") || contains(header, "ifstream")) return 2;
  if (!contains(internal, "resolve_asset_path") ||
      !contains(path, "valid_relative_asset_path") ||
      !contains(path, "path_within_root") ||
      !contains(path, "lexically_normal()") ||
      !contains(path, "if (!exists) return candidate") ||
      !contains(path, "fs::canonical(candidate, error)") ||
      !contains(path, "resolved asset path escapes")) return 3;
  if (!contains(source, "fs::file_size(") ||
      !contains(source, "file_size > options_.max_asset_bytes") ||
      !contains(source, "asset.bytes.resize(") ||
      source.find("file_size > options_.max_asset_bytes") >
          source.find("asset.bytes.resize(")) return 4;
  if (!contains(source, "std::ios::binary") ||
      !contains(source, "if (!fs::exists(status)) return std::nullopt") ||
      !contains(source, "std::ranges::sort(entries)") ||
      !contains(error, "asset_io_failed") ||
      !contains(error, "asset_too_large")) return 5;
  if (!contains(behavior, "test_loads_binary_empty_and_missing_assets") ||
      !contains(behavior, "test_rejects_escape_absolute_directory_and_oversize_paths") ||
      !contains(behavior, "test_lists_entries_stably_and_rejects_invalid_directories") ||
      !contains(behavior, "FileAssetSource({})") ||
      !contains(cleanliness, "cgpui/core/asset_source.hpp") ||
      !contains(prelude, "cgpui/core/asset_source.hpp")) return 6;
  if (!contains(previous, "Step 651 file-backed asset loading ") ||
      !contains(previous, "production behavior is next.") ||
      !contains(xmake, "target(\"phase_g_file_asset_source_test\")") ||
      !contains(xmake,
                "target(\"phase_g_file_asset_source_structure_test\")")) {
    return 7;
  }
  if (line_count(header) > 65 || line_count(internal) > 20 ||
      line_count(path) > 100 || line_count(source) > 135 ||
      line_count(behavior) > 140) return 8;

  constexpr const char* completion =
      "Phase G Step 651 adds root-confined file-backed AssetSource loading "
      "with binary and empty-file support, optional missing-file results, "
      "stable directory listing, canonical symlink escape protection, byte "
      "limits before allocation, and explicit invalid-path and I/O "
      "diagnostics. Step 652 PNG and JPEG decode boundary production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 9;
  }
  if (!contains(vocabulary, "`AssetSource`") ||
      !contains(vocabulary, "`FileAssetSource`") ||
      !contains(core, "root-confined file-backed asset loading") ||
      !contains(ledger_json, "\"phase_g_step_651_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 658 official image/GIF "
                "examples and asset closeout\"")) return 10;
  return 0;
}

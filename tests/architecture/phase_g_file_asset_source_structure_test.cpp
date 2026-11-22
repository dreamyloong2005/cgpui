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
  const std::string manifests = read_source("tests/assets/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &header, &error, &internal, &path, &source, &prelude, &cleanliness,
      &behavior, &manifests, &vocabulary, &core, &ledger_json};
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
  if (!contains(manifests, "\"file_asset_source_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_file_asset_source_structure_test.cpp\"")) {
    return 7;
  }
  if (line_count(header) > 65 || line_count(internal) > 20 ||
      line_count(path) > 100 || line_count(source) > 135 ||
      line_count(behavior) > 140) return 8;

  if (!contains(vocabulary, "`AssetSource`") ||
      !contains(vocabulary, "`FileAssetSource`") ||
      !contains(core, "root-confined file-backed asset loading") ||
      !contains(ledger_json, "\"phase_g_step_651_sources\"")) return 10;
  return 0;
}

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
  const std::string header =
      read_source("include/cgpui/core/asset_cache_key.hpp");
  const std::string source = read_source("src/core/asset_cache_key.cpp");
  const std::string behavior =
      read_source("tests/assets/asset_cache_key_test.cpp");
  const std::string core_cleanliness =
      read_source("tests/header_cleanliness/core_header_cleanliness.cpp");
  const std::string prelude = read_source("include/cgpui/prelude.hpp");
  const std::string prelude_cleanliness =
      read_source("tests/header_cleanliness/prelude_header_cleanliness.cpp");
  const std::string manifests = read_source("tests/assets/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &header, &source, &behavior, &core_cleanliness, &prelude,
      &prelude_cleanliness, &manifests, &vocabulary, &core, &ledger_json};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "struct AssetSourceId") ||
      !contains(header, "enum class AssetCacheKind") ||
      !contains(header, "struct AssetCacheKey") ||
      !contains(header, "struct AssetCacheKeyHash") ||
      !contains(header, "make_asset_cache_key(") ||
      contains(header, "<filesystem>") || contains(header, "std::hash")) {
    return 2;
  }
  if (!contains(source, "std::ranges::replace(portable, '\\\\', '/')") ||
      !contains(source, "path.lexically_normal().generic_string()") ||
      !contains(source, "component == \"..\"") ||
      !contains(source, "portable.find('\\0')") ||
      !contains(source, "portable.find(':')") ||
      !contains(source, "14695981039346656037ULL") ||
      !contains(source, "1099511628211ULL") ||
      !contains(source, "hash_u64(hash, path.size())")) return 3;
  if (!contains(behavior, "test_normalizes_paths_and_hashes_stably") ||
      !contains(behavior, "test_separates_source_kind_and_revision") ||
      !contains(behavior, "test_rejects_invalid_source_and_paths") ||
      !contains(behavior, "alternate_stream") ||
      !contains(behavior, "embedded_null") ||
      !contains(behavior, "std::unordered_map<cgpui::AssetCacheKey") ||
      !contains(core_cleanliness, "header_asset_key->stable_hash()") ||
      !contains(prelude_cleanliness, "prelude_asset_key->revision")) return 4;
  if (!contains(prelude, "cgpui/core/asset_cache_key.hpp") ||
      !contains(manifests, "\"asset_cache_key_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_asset_cache_key_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(header) > 60 || line_count(source) > 110 ||
      line_count(behavior) > 100) return 6;

  if (!contains(vocabulary, "`AssetCacheKey`") ||
      !contains(vocabulary, "`make_asset_cache_key(...)`") ||
      !contains(core, "Stable decoded-asset cache keys") ||
      !contains(ledger_json, "\"phase_g_step_655_sources\"")) return 8;
  return 0;
}

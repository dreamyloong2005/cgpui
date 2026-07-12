#include "cgpui/core/asset_cache_key.hpp"

#include <algorithm>
#include <filesystem>
#include <utility>

namespace cgpui {
namespace {

namespace fs = std::filesystem;

Error invalid_cache_key(std::string message) {
  return Error{.code = ErrorCode::invalid_argument, .message = std::move(message)};
}

Result<std::string> normalize_cache_path(std::string_view requested) {
  std::string portable(requested);
  std::ranges::replace(portable, '\\', '/');
  if (portable.empty() || portable.front() == '/' ||
      portable.find('\0') != std::string::npos ||
      portable.find(':') != std::string::npos) {
    return std::unexpected(invalid_cache_key(
        "asset cache path must be a non-empty relative path"));
  }

  const fs::path path(portable);
  if (path.is_absolute() || path.has_root_name() || path.has_root_directory()) {
    return std::unexpected(invalid_cache_key(
        "asset cache path must not contain a filesystem root"));
  }
  for (const fs::path& component : path) {
    if (component == "..") {
      return std::unexpected(invalid_cache_key(
          "asset cache path must stay within its source"));
    }
  }
  std::string normalized = path.lexically_normal().generic_string();
  if (normalized.empty() || normalized == ".") {
    return std::unexpected(invalid_cache_key(
        "asset cache path must identify an asset"));
  }
  return normalized;
}

void hash_byte(std::uint64_t& hash, std::uint8_t value) {
  hash ^= value;
  hash *= 1099511628211ULL;
}

void hash_u64(std::uint64_t& hash, std::uint64_t value) {
  for (unsigned int shift = 0; shift < 64; shift += 8) {
    hash_byte(hash, static_cast<std::uint8_t>(value >> shift));
  }
}

} // namespace

std::uint64_t AssetCacheKey::stable_hash() const {
  std::uint64_t hash = 14695981039346656037ULL;
  hash_u64(hash, source_id.value);
  hash_byte(hash, static_cast<std::uint8_t>(kind));
  hash_u64(hash, revision);
  hash_u64(hash, path.size());
  for (const unsigned char value : path) hash_byte(hash, value);
  return hash;
}

std::size_t AssetCacheKeyHash::operator()(const AssetCacheKey& key) const {
  return static_cast<std::size_t>(key.stable_hash());
}

Result<AssetCacheKey> make_asset_cache_key(
    AssetSourceId source_id,
    std::string_view path,
    AssetCacheKind kind,
    std::uint64_t revision) {
  if (source_id.value == 0) {
    return std::unexpected(invalid_cache_key(
        "asset cache source id must be non-zero"));
  }
  Result<std::string> normalized = normalize_cache_path(path);
  if (!normalized) return std::unexpected(normalized.error());
  return AssetCacheKey{
      .source_id = source_id,
      .path = std::move(*normalized),
      .kind = kind,
      .revision = revision,
  };
}

} // namespace cgpui

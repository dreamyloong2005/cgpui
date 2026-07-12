#pragma once

#include "cgpui/core/error.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace cgpui {

struct AssetSourceId {
  std::uint64_t value = 0;

  friend bool operator==(AssetSourceId, AssetSourceId) = default;
};

enum class AssetCacheKind : std::uint8_t {
  bytes,
  raster_image,
  animated_gif,
  svg_document,
};

struct AssetCacheKey {
  AssetSourceId source_id;
  std::string path;
  AssetCacheKind kind = AssetCacheKind::bytes;
  std::uint64_t revision = 0;

  [[nodiscard]] std::uint64_t stable_hash() const;

  friend bool operator==(const AssetCacheKey&, const AssetCacheKey&) = default;
};

struct AssetCacheKeyHash {
  [[nodiscard]] std::size_t operator()(const AssetCacheKey& key) const;
};

[[nodiscard]] Result<AssetCacheKey> make_asset_cache_key(
    AssetSourceId source_id,
    std::string_view path,
    AssetCacheKind kind,
    std::uint64_t revision = 0);

} // namespace cgpui

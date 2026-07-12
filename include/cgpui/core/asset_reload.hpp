#pragma once

#include "cgpui/core/asset_cache_key.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace cgpui {

struct AssetReloadInvalidation {
  AssetSourceId source_id;
  std::string path;
  std::uint64_t previous_revision = 0;
  std::uint64_t new_revision = 0;
};

struct AssetSourceInvalidation {
  AssetSourceId source_id;
  std::size_t invalidated_assets = 0;
};

struct AssetReloadSnapshot {
  std::size_t tracked_assets = 0;
  std::size_t invalidation_count = 0;
  std::size_t source_invalidation_count = 0;
};

class AssetReloadState {
 public:
  AssetReloadState();
  ~AssetReloadState();

  AssetReloadState(const AssetReloadState&) = delete;
  AssetReloadState& operator=(const AssetReloadState&) = delete;

  [[nodiscard]] Result<AssetCacheKey> key(
      AssetSourceId source_id,
      std::string_view path,
      AssetCacheKind kind);
  [[nodiscard]] Result<AssetReloadInvalidation> invalidate(
      AssetSourceId source_id,
      std::string_view path);
  [[nodiscard]] Result<AssetSourceInvalidation> invalidate_source(
      AssetSourceId source_id);
  [[nodiscard]] AssetReloadSnapshot snapshot() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace cgpui

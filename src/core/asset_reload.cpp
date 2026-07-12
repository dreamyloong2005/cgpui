#include "cgpui/core/asset_reload.hpp"

#include <limits>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace cgpui {
namespace {

Error reload_error(ErrorCode code, std::string message) {
  return Error{.code = code, .message = std::move(message)};
}

Result<AssetCacheKey> reload_identity(
    AssetSourceId source_id,
    std::string_view path) {
  return make_asset_cache_key(
      source_id, path, AssetCacheKind::bytes, 0);
}

} // namespace

class AssetReloadState::Impl {
 public:
  mutable std::mutex mutex;
  std::unordered_map<AssetCacheKey, std::uint64_t, AssetCacheKeyHash> revisions;
  std::size_t invalidation_count = 0;
  std::size_t source_invalidation_count = 0;
};

AssetReloadState::AssetReloadState() : impl_(std::make_unique<Impl>()) {}

AssetReloadState::~AssetReloadState() = default;

Result<AssetCacheKey> AssetReloadState::key(
    AssetSourceId source_id,
    std::string_view path,
    AssetCacheKind kind) {
  Result<AssetCacheKey> identity = reload_identity(source_id, path);
  if (!identity) return std::unexpected(identity.error());
  std::scoped_lock lock(impl_->mutex);
  const auto [entry, inserted] = impl_->revisions.try_emplace(*identity, 0);
  (void)inserted;
  identity->kind = kind;
  identity->revision = entry->second;
  return identity;
}

Result<AssetReloadInvalidation> AssetReloadState::invalidate(
    AssetSourceId source_id,
    std::string_view path) {
  Result<AssetCacheKey> identity = reload_identity(source_id, path);
  if (!identity) return std::unexpected(identity.error());
  std::scoped_lock lock(impl_->mutex);
  std::uint64_t& revision = impl_->revisions.try_emplace(*identity, 0).first->second;
  if (revision == std::numeric_limits<std::uint64_t>::max()) {
    return std::unexpected(reload_error(
        ErrorCode::asset_revision_exhausted,
        "asset revision counter is exhausted"));
  }
  const std::uint64_t previous = revision++;
  impl_->invalidation_count += 1;
  return AssetReloadInvalidation{
      .source_id = source_id,
      .path = std::move(identity->path),
      .previous_revision = previous,
      .new_revision = revision,
  };
}

Result<AssetSourceInvalidation> AssetReloadState::invalidate_source(
    AssetSourceId source_id) {
  if (source_id.value == 0) {
    return std::unexpected(reload_error(
        ErrorCode::invalid_argument,
        "asset reload source id must be non-zero"));
  }
  std::scoped_lock lock(impl_->mutex);
  for (const auto& [key, revision] : impl_->revisions) {
    if (key.source_id == source_id &&
        revision == std::numeric_limits<std::uint64_t>::max()) {
      return std::unexpected(reload_error(
          ErrorCode::asset_revision_exhausted,
          "asset revision counter is exhausted"));
    }
  }

  std::size_t invalidated = 0;
  for (auto& [key, revision] : impl_->revisions) {
    if (key.source_id != source_id) continue;
    revision += 1;
    invalidated += 1;
  }
  impl_->invalidation_count += invalidated;
  impl_->source_invalidation_count += 1;
  return AssetSourceInvalidation{
      .source_id = source_id,
      .invalidated_assets = invalidated,
  };
}

AssetReloadSnapshot AssetReloadState::snapshot() const {
  std::scoped_lock lock(impl_->mutex);
  return AssetReloadSnapshot{
      .tracked_assets = impl_->revisions.size(),
      .invalidation_count = impl_->invalidation_count,
      .source_invalidation_count = impl_->source_invalidation_count,
  };
}

} // namespace cgpui

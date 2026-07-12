#include "cgpui/core/asset_cache_key.hpp"

#include <string>
#include <unordered_map>

namespace {

int test_normalizes_paths_and_hashes_stably() {
  const auto first = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{7}, "icons\\./toolbar//save.png",
      cgpui::AssetCacheKind::raster_image, 3);
  const auto second = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{7}, "icons/toolbar/save.png",
      cgpui::AssetCacheKind::raster_image, 3);
  if (!first || !second || *first != *second ||
      first->path != "icons/toolbar/save.png" ||
      first->stable_hash() != second->stable_hash() ||
      cgpui::AssetCacheKeyHash{}(*first) != first->stable_hash()) {
    return 10;
  }
  std::unordered_map<cgpui::AssetCacheKey, int, cgpui::AssetCacheKeyHash> cache;
  cache.emplace(*first, 42);
  return cache.at(*second) == 42 ? 0 : 11;
}

int test_separates_source_kind_and_revision() {
  const auto base = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{1}, "image.png",
      cgpui::AssetCacheKind::raster_image, 0);
  const auto source = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{2}, "image.png",
      cgpui::AssetCacheKind::raster_image, 0);
  const auto kind = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{1}, "image.png",
      cgpui::AssetCacheKind::animated_gif, 0);
  const auto revision = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{1}, "image.png",
      cgpui::AssetCacheKind::raster_image, 1);
  if (!base || !source || !kind || !revision || *base == *source ||
      *base == *kind || *base == *revision) {
    return 20;
  }
  return base->stable_hash() != source->stable_hash() &&
                 base->stable_hash() != kind->stable_hash() &&
                 base->stable_hash() != revision->stable_hash()
             ? 0
             : 21;
}

int test_rejects_invalid_source_and_paths() {
  const auto missing_source = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{}, "image.png", cgpui::AssetCacheKind::bytes);
  const auto empty = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{1}, "", cgpui::AssetCacheKind::bytes);
  const auto absolute = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{1}, "/image.png", cgpui::AssetCacheKind::bytes);
  const auto drive = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{1}, "C:\\image.png", cgpui::AssetCacheKind::bytes);
  const auto escape = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{1}, "icons/../image.png",
      cgpui::AssetCacheKind::bytes);
  const auto alternate_stream = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{1}, "image.png:dark", cgpui::AssetCacheKind::bytes);
  const auto embedded_null = cgpui::make_asset_cache_key(
      cgpui::AssetSourceId{1}, std::string{"a\0b", 3},
      cgpui::AssetCacheKind::bytes);
  const auto invalid = [](const auto& result) {
    return !result && result.error().code == cgpui::ErrorCode::invalid_argument;
  };
  return invalid(missing_source) && invalid(empty) && invalid(absolute) &&
                 invalid(drive) && invalid(escape) &&
                 invalid(alternate_stream) && invalid(embedded_null)
             ? 0
             : 30;
}

} // namespace

int main() {
  if (const int result = test_normalizes_paths_and_hashes_stably()) return result;
  if (const int result = test_separates_source_kind_and_revision()) return result;
  return test_rejects_invalid_source_and_paths();
}

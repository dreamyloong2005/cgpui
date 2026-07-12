#include "cgpui/core/asset_reload.hpp"

#include <thread>
#include <vector>

namespace {

int test_invalidates_all_variants_for_one_asset() {
  cgpui::AssetReloadState reloads;
  const auto image = reloads.key(
      cgpui::AssetSourceId{1}, "images/logo.png",
      cgpui::AssetCacheKind::raster_image);
  const auto bytes = reloads.key(
      cgpui::AssetSourceId{1}, "images/logo.png",
      cgpui::AssetCacheKind::bytes);
  const auto other = reloads.key(
      cgpui::AssetSourceId{1}, "images/other.png",
      cgpui::AssetCacheKind::raster_image);
  const auto invalidation =
      reloads.invalidate(cgpui::AssetSourceId{1}, "images/logo.png");
  const auto next_image = reloads.key(
      cgpui::AssetSourceId{1}, "images/logo.png",
      cgpui::AssetCacheKind::raster_image);
  const auto next_bytes = reloads.key(
      cgpui::AssetSourceId{1}, "images/logo.png",
      cgpui::AssetCacheKind::bytes);
  const auto same_other = reloads.key(
      cgpui::AssetSourceId{1}, "images/other.png",
      cgpui::AssetCacheKind::raster_image);
  return image && bytes && other && invalidation && next_image && next_bytes &&
                 same_other && image->revision == 0 && bytes->revision == 0 &&
                 invalidation->previous_revision == 0 &&
                 invalidation->new_revision == 1 && next_image->revision == 1 &&
                 next_bytes->revision == 1 && *image != *next_image &&
                 *bytes != *next_bytes && *other == *same_other
             ? 0
             : 10;
}

int test_invalidates_a_tracked_source_in_one_operation() {
  cgpui::AssetReloadState reloads;
  (void)reloads.key(
      cgpui::AssetSourceId{5}, "a.png", cgpui::AssetCacheKind::raster_image);
  (void)reloads.key(
      cgpui::AssetSourceId{5}, "b.gif", cgpui::AssetCacheKind::animated_gif);
  (void)reloads.key(
      cgpui::AssetSourceId{6}, "c.svg", cgpui::AssetCacheKind::svg_document);
  const auto invalidation = reloads.invalidate_source(cgpui::AssetSourceId{5});
  const auto a = reloads.key(
      cgpui::AssetSourceId{5}, "a.png", cgpui::AssetCacheKind::raster_image);
  const auto b = reloads.key(
      cgpui::AssetSourceId{5}, "b.gif", cgpui::AssetCacheKind::animated_gif);
  const auto c = reloads.key(
      cgpui::AssetSourceId{6}, "c.svg", cgpui::AssetCacheKind::svg_document);
  return invalidation && invalidation->invalidated_assets == 2 && a && b && c &&
                 a->revision == 1 && b->revision == 1 && c->revision == 0
             ? 0
             : 20;
}

int test_serializes_concurrent_invalidations() {
  cgpui::AssetReloadState reloads;
  constexpr int thread_count = 8;
  constexpr int invalidations_per_thread = 100;
  std::vector<std::thread> workers;
  for (int thread = 0; thread < thread_count; ++thread) {
    workers.emplace_back([&] {
      for (int index = 0; index < invalidations_per_thread; ++index) {
        (void)reloads.invalidate(cgpui::AssetSourceId{9}, "shared.png");
      }
    });
  }
  for (auto& worker : workers) worker.join();
  const auto key = reloads.key(
      cgpui::AssetSourceId{9}, "shared.png",
      cgpui::AssetCacheKind::raster_image);
  const cgpui::AssetReloadSnapshot snapshot = reloads.snapshot();
  return key && key->revision == 800 && snapshot.tracked_assets == 1 &&
                 snapshot.invalidation_count == 800 &&
                 snapshot.source_invalidation_count == 0
             ? 0
             : 30;
}

int test_rejects_invalid_identity() {
  cgpui::AssetReloadState reloads;
  const auto key = reloads.key(
      cgpui::AssetSourceId{}, "asset.png",
      cgpui::AssetCacheKind::raster_image);
  const auto invalidation =
      reloads.invalidate(cgpui::AssetSourceId{1}, "../asset.png");
  const auto source = reloads.invalidate_source(cgpui::AssetSourceId{});
  return !key && !invalidation && !source &&
                 key.error().code == cgpui::ErrorCode::invalid_argument &&
                 invalidation.error().code ==
                     cgpui::ErrorCode::invalid_argument &&
                 source.error().code == cgpui::ErrorCode::invalid_argument
             ? 0
             : 40;
}

} // namespace

int main() {
  if (const int result = test_invalidates_all_variants_for_one_asset()) {
    return result;
  }
  if (const int result = test_invalidates_a_tracked_source_in_one_operation()) {
    return result;
  }
  if (const int result = test_serializes_concurrent_invalidations()) return result;
  return test_rejects_invalid_identity();
}

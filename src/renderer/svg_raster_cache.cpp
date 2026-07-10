#include "cgpui/renderer/svg_raster_cache.hpp"

#include <string_view>
#include <utility>

namespace cgpui {

bool SvgRasterCacheKey::matches(
    const SvgRasterizationRequest& request) const {
  return asset_id == request.asset_id &&
         logical_size.width == request.logical_size.width &&
         logical_size.height == request.logical_size.height &&
         scale == request.scale.value &&
         std::string_view(svg_source) == request.svg_source;
}

bool SvgRasterCacheLookup::ready() const {
  return status == SvgRasterizationStatus::ready && result != nullptr &&
         result->ready();
}

SvgRasterCacheLookup SvgRasterCache::rasterize(
    const SvgRasterizationRequest& request) {
  lookup_count_ += 1;
  for (const Entry& entry : entries_) {
    if (entry.key.matches(request)) {
      hit_count_ += 1;
      return SvgRasterCacheLookup{
          .status = entry.result.status,
          .result = &entry.result,
          .cache_hit = true,
      };
    }
  }

  miss_count_ += 1;
  SvgRasterizationResult result = rasterize_svg(request);
  if (!result.ready()) {
    return SvgRasterCacheLookup{.status = result.status};
  }

  entries_.push_back(Entry{
      .key =
          SvgRasterCacheKey{
              .asset_id = request.asset_id,
              .logical_size = request.logical_size,
              .scale = request.scale.value,
              .svg_source = std::string(request.svg_source),
          },
      .result = std::move(result),
  });
  const SvgRasterizationResult& stored = entries_.back().result;
  return SvgRasterCacheLookup{
      .status = stored.status,
      .result = &stored,
  };
}

std::size_t SvgRasterCache::entry_count() const {
  return entries_.size();
}

std::size_t SvgRasterCache::lookup_count() const {
  return lookup_count_;
}

std::size_t SvgRasterCache::hit_count() const {
  return hit_count_;
}

std::size_t SvgRasterCache::miss_count() const {
  return miss_count_;
}

void SvgRasterCache::clear() {
  entries_.clear();
}

} // namespace cgpui
